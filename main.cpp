#include "widget.h"
#include "Misc.h"
#include <stdio.h>
#include <dirent.h>
#include <iostream>
#include <limits>
#include <cstddef>
#include "Functions.h"
#include "aimbot.h"
#include <QApplication>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QColorDialog>
#include <thread>                   // fix these damn imports so they are grouped by libs and actual feature headers
#include <fstream>
#include "paint.h"
#include "offset_fetch\offsets.hpp"   // using pyfetcher
#include "Structs.h"
#include "entity.h"
#include "esp.h"
#include <QStyleFactory>
#include "stylesheet.h"
#include <QTimer>
#include <QList>
#include <QFile>

#include "widget.h"
#include "ui_widget.h"
#include "radarwidget.h"
#include "ui_radarwidget.h"

/*          =-=-=<[ CS:GO external cheat ]>=-=-=
 *        written by https://www.github.com/skajuks
 *                      2021
 *
 *KNOWN ISSUES:
 *sendpackets offset not read correctly from memory.
 *
 *IDEAS
 *
 *Option for flash max value to be changed ( make look legit )        kinda done, looks goofy tho
 *Aimbot working if enemy dormant
 *Show viewangles of enemy
 *Can change aim bone
 *Can switch between teams for blockbot
 *Non-head blockbot
 *FakeLag if velocity > 0
 *Show enemy team on radar
 *
*/

#undef max // C macro for max variable from math.h - have to undef because class::max() would paste macro content at "max" (class:((a)>(b)?(a):(b)))
#define LEFT_MOUSE_BUTTON 0x01
#define NDEBUG 0
#define ERROR_LOG(what) std::cout << "[Error]" << std::endl << what << std::endl << __FILE__ << std::endl << __PRETTY_FUNCTION__ << std::endl << __LINE__ << std::endl;

ClientInfo ci[64];  // ci[0] = localplayer
Entity e[64];       // e[0] = localplayer
toggleStateData stateData[6];

class Glow;
Paint Paint();

bool  toggleHealthGlow  = false;
bool  ToggleNoFlash     = false;
bool  toggleAimbot      = false;
int   aimbot_fov        =     1;
bool  night_state       = false;
float aimbot_smooth     =   1.f;                            // fix this fucking shit, fix camel cases being applied on vars
float aimbot_recoil     =   1.f;
float glow_alpha        =   1.f;
int   flashAmount       =     0;
float nightmode_amount  = 0.04f;
int   aimbot_on_key     =  LEFT_MOUSE_BUTTON;                            // mouse 1 default
bool  enable_silent     = false;
bool  jump_shot         = false;
bool  toggle_aimbot_key = false;
bool  blockbot_enabled  = false;
bool  blockbot_enemy_enabled = false;
bool  blocked           = false;
bool  door_spammer      = false;
bool  thirdperson       = false;
bool  dmexploit         = false;
bool  esp_master_state  = false;
bool  autoaccpet_clicked= false;
bool  radar_enabled     = false;
bool  fakelag_enabled   = false;
bool  target_teammates  = false;
bool  radar_opened      = false;
const char* skybox_name;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

RadarWidget::RadarWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RadarWidget)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    QPixmap img("maps/cs_office.png");
    scene->addPixmap(img);
    ui->graphicsView->setScene(scene);
    blackPen.setWidth(1);
}

RadarWidget::~RadarWidget()
{
    delete ui;
}

std::vector<playerInfoStacked> whitelist;
std::vector<playerInfoStacked> player_list;

std::vector<VECTOR2>* TeamPlayers = new std::vector<VECTOR2>;
std::vector<VECTOR2>* EnemyPlayers = new std::vector<VECTOR2>;

std::array<char, 64> map;
std::string mapName;
struct::map current_map = {};

ColorRGB red = {255,0,0};
ColorRGB green = {0,255,0};
ColorRGB blue = {0,0,255};
ColorRGB purple = {255,0,255};

extern int aimfov;
void getClosestEntityIndex(int &entityIndex, float &oldx, float &oldy, int &targetIndex);

QApplication* app;
Widget* window;
RadarWidget* radar_window;

int main(int argc, char** argv) {
    app = new QApplication(argc, argv);
    window = new Widget;
    radar_window = new RadarWidget;
    qApp->setStyle(QStyleFactory::create("Fusion"));
    qApp->setStyleSheet(style_sheet);

    window->show();
    whitelist.reserve(50);
    player_list.reserve(50);
    // allocate space for vector
    TeamPlayers->reserve(20);
    EnemyPlayers->reserve(20);
    // add bones and entity selector function choices to ui

    // open and read config file
    std::ifstream stream("cfg/cfg.txt");
    std::string line;
    if(!stream){
        ERROR_LOG("Cannot open cfg file!")
    }

    while(std::getline(stream, line))
    {
        whitelist.push_back({line, " "});
        window->ui->player_list_whitelist->addItem(QString(line.c_str()));
    }

    //whitelist.push_back({"Frynights","STEAM_1:1:89702025"});
    //whitelist.push_back({"Big Ounce","STEAM_0:0:101827133"});
    //(auto a: whitelist)
        //window->ui->player_list_whitelist->addItem(QString::fromStdString(a.name));

    for(auto item: skybox_array){
        window->ui->skybox_list->addItem(QString(item));
    }
    window->ui->aimbot_calc->addItems({"Angle fov", "Distance", "Crosshair fov"});
    window->ui->aimbot_calc->setCurrentRow(0);
    window->ui->aimBonesList->addItems({"8 - head", "6 - upper chest", "5 - lower chest", "9 - neck", "2 - pelvis"});
    window->ui->aimBonesList->setCurrentRow(0);

    // here goes static features that doesn't need updates
#if NDEBUG
    Glow::setBrightness();

    Misc::setNightmodeAmount(0.09f);

    //std::thread(ESP::run).detach();     // d9x9 thread
#endif
#ifdef MISCITEMS
    std::thread (Misc::miscItemsThreaded, e[0], ci[0]).detach(); // thread for dm invounerability exploit and perfect nade
#endif
    //std::thread (Misc::fakeLagThreaded, e[0], fakelag_enabled).detach();  // thread used for fakelag

    QTimer *timer = new QTimer(window);
    QObject::connect(timer, &QTimer::timeout, [&](){

        int entityIndex = 1;
        float oldx = std::numeric_limits<float>::max();
        float oldy = std::numeric_limits<float>::max();
        float bestBlockDist = 250.f;
        int32_t targetIndex;
        int32_t blockTargetIndex;
        int32_t aliveEntity = 0;
        uintptr_t glowObjectManager = Memory.readMem<uintptr_t>(pOffsets.dwGlowObjectManager);
        aimbotVariables variables;

        // Under here goes everything that needs to be updated for every entity except local player

        Memory.readMemTo<ClientInfo>(pOffsets.dwLocalPlayer, &ci[0]);
        Memory.readMemTo<Entity>(ci[0].entity, &e[0]);

        uintptr_t clientState = Functions::getClientState();
        map = Memory.readMem<std::array<char, 64>>(clientState + pyfetcher::dwClientState_Map);
        for(size_t i = 0; i < map_list.size(); i++)
        {
            if(map_list[i].map_name == std::string(map.data()))
                current_map = map_list[i];
        }
        do {

            if(entityIndex >= 64) break;

            Memory.readMemTo<ClientInfo>(pOffsets.dwEntityList + entityIndex * 0x10, &ci[entityIndex]);
            Memory.readMemTo<Entity>(ci[entityIndex].entity, &e[entityIndex]);

            if(e[entityIndex].health > 0 && ci[entityIndex].entity) { // checks if entity exists and is alive

                aliveEntity++;

                if(e[entityIndex].team == e[0].team){

                    if(blockbot_enabled){
                        float dist = Aim::getClosestEntityByDistance(&e[0].vecOrigin, &e[entityIndex].vecOrigin);     //add toggle for enemy team
                        if(!Math::equalVector(&e[0].vecOrigin, &e[entityIndex].vecOrigin))
                            if(dist < bestBlockDist && entityIndex != 0){
                                bestBlockDist = dist;
                                blockTargetIndex = entityIndex;
                            }
                    }
                    Glow::ProcessEntityTeam(ci[entityIndex], glowObjectManager, blockTargetIndex == entityIndex ? true : false);
                    TeamPlayers->push_back(Math::calculateOpenGLXY(e[entityIndex].vecOrigin));
                }
                else {

                    if(radar_enabled)
                        Memory.writeMem<bool>(ci[0].entity + pNetVars.m_bSpotted, true);    // make this look nicer :)

                    if(toggleAimbot && !target_teammates){
                        getClosestEntityIndex(entityIndex, oldx, oldy, targetIndex);
                    }
                    Glow::ProcessEntityEnemy(ci[entityIndex], e[entityIndex], glowObjectManager, targetIndex == entityIndex ? true : false);
                    EnemyPlayers->push_back(Math::calculateOpenGLXY(e[entityIndex].vecOrigin));
                }
                if(toggleAimbot && target_teammates){
                    getClosestEntityIndex(entityIndex, oldx, oldy, targetIndex);
                }
                player_info player;
                Functions::getPlayerInfo(entityIndex, player, clientState);
                bool inList = 0;
                for(size_t i = 0; i < player_list.size(); i++){
                    if(player_list[i].id == player.steam_id)
                        inList = 1;
                }
                if(!inList){
                    player_list.push_back({player.name, player.steam_id});
                    window->ui->player_list_widget->addItem(QString::fromUtf8(std::string(player.name).c_str()));
                }
                for(auto& p: whitelist){
                    if (p.id == player.steam_id)
                        Glow::ProcessTargetEntity(ci[entityIndex], glowObjectManager, purple);
                }
            }
        } while(ci[entityIndex++].nextEntity);

        //radar_window->scene->addRect(e.x,std::abs(e.y),10,10, radar_window->blackPen, radar_window->redBrush);

        EnemyPlayers->clear();
        TeamPlayers->clear();
        // Under here goes everything that needs to be updated for local player only!

        // blockbot      MOVE TO A FUNCTION TO CLEAN UP MAIN

        if((blockbot_enabled || blockbot_enemy_enabled) && GetAsyncKeyState(0x58) && blockTargetIndex){ // x key
            blocked = true;
            VECTOR3 currentAngle = Math::PlayerAngles();
            VECTOR3 bestPos = e[blockTargetIndex].vecOrigin - e[0].vecOrigin;
            VECTOR3 aimAngle(currentAngle.x, RAD2DEG(atan2(bestPos.y, bestPos.x)), currentAngle.z);
            Math::ClampAngles(&aimAngle.x, &aimAngle.y);

            VECTOR2 svForward, svRight;
            Math::AngleVectors(currentAngle, &svForward, &svRight, nullptr);
            Math::vectorNormalize(svForward);
            Math::vectorNormalize(svRight);

            VECTOR2 clForward, clRight;
            Math::AngleVectors(aimAngle, &clForward, &clRight, nullptr);
            Math::vectorNormalize(clForward);
            Math::vectorNormalize(clRight);

            float divider = (svForward.x * svRight.y - svForward.y * svRight.x);

            if(divider != 0.f){
                float fmove = -((clForward.x*svRight.y - clForward.y*svRight.x)*450.f) / divider;
                float smove = ((clForward.x*svForward.y - clForward.y*svForward.x)*450.f) / divider;

                if(fmove > 0.f){
                    Memory.write<int8_t>(pOffsets.forward, 5);
                } else if (fmove < 0.f){
                    Memory.write<int8_t>(pOffsets.back, 5);
                }
                if(smove > 0.f){
                    Memory.write<int8_t>(pOffsets.right, 5);
                } else if (smove < 0.f){
                    Memory.write<int8_t>(pOffsets.left, 5);
                }
            }
        }

        if(blocked && (blockbot_enabled || blockbot_enemy_enabled) && !GetAsyncKeyState(0x58)){
            Memory.write<int8_t>(pOffsets.right, 4);
            Memory.write<int8_t>(pOffsets.forward, 4);
            Memory.write<int8_t>(pOffsets.back, 4);
            Memory.write<int8_t>(pOffsets.left, 4);
            blocked = false;
        }

        if(door_spammer && GetAsyncKeyState(VK_HOME)){
            Misc::doorSpammer(5);
            Sleep(13);
            Misc::doorSpammer(4);
            Sleep(13);
        }

        Misc::setBhop(e[0]);
        Misc::setNoFlash(ci[0]);

        if(GetAsyncKeyState(0x56) && toggleAimbot && targetIndex && jump_shot) {
            if(e[0].vecVelocity.z < 15 && e[0].vecVelocity.z > 0){
                variables = Aim::executeAimbot(ci[targetIndex], e[0], ci[0]);
            }
        }
        if(GetAsyncKeyState(LEFT_MOUSE_BUTTON) && toggleAimbot && targetIndex) {
            if(toggle_aimbot_key) {
                if (GetAsyncKeyState(0x43)){
                    variables = Aim::executeAimbot(ci[targetIndex], e[0], ci[0]);
                }
            } else {
                variables = Aim::executeAimbot(ci[targetIndex], e[0], ci[0]);        // execute code only if aimbot is actually enabled
            }
        }
    });
    timer->start(1);

    return app->exec();
}

void getClosestEntityIndex(int &entityIndex, float &oldx, float &oldy, int &targetIndex)    // should not be here
{
    VECTOR2 newDistance = Aim::getClosestEntity(e[0], ci[entityIndex], ci[0]);
    if(newDistance.x < oldx && newDistance.y < oldy && newDistance.x <= aimfov && newDistance.y <= aimfov){
        oldx = newDistance.x;
        oldy = newDistance.y;
        targetIndex = entityIndex;
    }
}

void Widget::on_radar_toggle_clicked()
{
    radar_opened = !radar_opened;
    ui->radar_toggle->setText(radar_opened ? QString("Close ext radar") : QString("Open ext radar"));
    radar_opened ? radar_window->show() : radar_window->hide();
}

void Widget::on_player_list_widget_itemDoubleClicked(QListWidgetItem *item)
{
    std::string selected = item->text().toStdString().c_str();
    for (auto& a: whitelist){
        if(strcmp(a.name.c_str(),selected.c_str()) == 0){
            return;
        }
    }
    for (auto& a: player_list){
        if(strcmp(a.name.c_str(),selected.c_str()) == 0){
            whitelist.push_back({a.name, a.id});
            ui->player_list_whitelist->addItem(QString::fromStdString(a.name));
        }
    }
}

void Widget::on_player_list_whitelist_itemDoubleClicked(QListWidgetItem *item)
{
    std::string selected = item->text().toStdString().c_str();
    for(auto& a: whitelist){
        size_t i = &a - &whitelist.front();
        if(strcmp(a.name.c_str(),selected.c_str()) == 0){

            whitelist.erase(whitelist.begin() + i);
            std::cout << a.id << std::endl;
            ui->player_list_whitelist->removeItemWidget(item);
            delete item;
        }
    }
}

void Widget::on_ffa_mode_enable_stateChanged(int arg1)
{
    target_teammates = !target_teammates;
}

void Widget::on_dormant_enable_stateChanged(int arg1)
{
    Aim::enableDormantMode();
}

void Widget::on_flash_slider_valueChanged(int value)
{
    Misc::setNoFlashAmount(float(value));
    ui->flash_value->setText(QString::number(value));
}

void Widget::on_enable_radio_stateChanged(int arg1)
{
    Misc::setRadioSpamEnabled();
}

void Widget::on_fakelag_enable_stateChanged(int arg1)
{
    fakelag_enabled = !fakelag_enabled;
}

void Widget::on_radar_enable_stateChanged(int arg1)
{
    radar_enabled = !radar_enabled;
}

void Widget::on_enable_autoaccept_clicked()
{
    autoaccpet_clicked = true;
    Misc::setAutoAcceptEnabled();
}

void Widget::on_perfect_nade_enable_stateChanged(int arg1){
    Misc::setPerfectNadeEnabled();
}

void Widget::on_dm_exploit_enable_stateChanged(int arg1){
    Misc::setDMExploitEnabled(dmexploit);
}

void Widget::on_thirdperson_toggle_stateChanged(int arg1){  // doens't work, must be updated in loop
    thirdperson = !thirdperson;
    //if(thirdperson)
        //Memory.writeMem<int>(ci[0].entity + m_iObserverMode, 1);
    //else
        //Memory.writeMem<int>(ci[0].entity + m_iObserverMode, 0);
}

void Widget::on_skybox_list_itemClicked(QListWidgetItem *item){
    skybox_name = item->text().toStdString().c_str();
    Functions::loadSkybox(skybox_name);
}

void Widget::on_testCommand_clicked(){
    Functions::clientCmd_Unrestricted("say \"Lmao es varu rakstit chata izmantojot cheatu un executot console komandas xDDDD\"");
}

void Widget::on_blockbot_enable_stateChanged(int arg1)
{
    blockbot_enabled = !blockbot_enabled;
}

void Widget::on_doorspammer_enable_stateChanged(int arg1)
{
    door_spammer = !door_spammer;
}

void Widget::on_toggle_aimbot_on_key_stateChanged(int arg1)
{
    toggle_aimbot_key = !toggle_aimbot_key;
}

void Widget::on_jump_shot_enable_stateChanged(int arg1)
{
    jump_shot = !jump_shot;
}

void Widget::on_esp_snapline_color_stateChanged(int arg1)
{
    QColor color = QColorDialog::getColor(Qt::blue, this);
    ui->SNAPCOL->setStyleSheet(QString("QPushButton{background-color: %1}").arg(color.name()));
    Glow::colorChanged(1, 255, color.red(), color.green(), color.blue());
}

void Widget::on_esp_box_color_stateChanged(int arg1)
{
    QColor color = QColorDialog::getColor(Qt::blue, this);
    ui->ESPCOL->setStyleSheet(QString("QPushButton{background-color: %1}").arg(color.name()));
    Glow::colorChanged(2, 255, color.red(), color.green(), color.blue());
}

void Widget::on_esp_health_stateChanged(int arg1)
{
    Glow::StateChanged(5); // enable esp by health
}

void Widget::on_esp_snaplines_stateChanged(int arg1)
{
    Glow::StateChanged(4); // enable snaplines
}

void Widget::on_esp_name_weapon_stateChanged(int arg1)
{
    Glow::StateChanged(3); // esp health and name
}

void Widget::on_enable_boxes_stateChanged(int arg1)
{
    Glow::StateChanged(2); // esp boxes
}

void Widget::on_esp_enable_stateChanged(int arg1)
{
    esp_master_state = !esp_master_state;
    Glow::StateChanged(esp_master_state); // esp master
    Paint::toggleEspMaster(esp_master_state);
}

void Widget::on_fakelag_slider_valueChanged(int value)
{
    //FakeLag::setFakelag(value);
    ui->fakelag_val->setText(QString::number(value));
}

void Widget::on_nightmode_slider_valueChanged(int value)
{
    float v = (float)value / 100;
    std::cout << v << std::endl;
    //Flash::setNightmodeAmount(v);
}

void Widget::on_chams_bright_slider_valueChanged(int value)
{
    Glow::brightnessStateChanged(true,(float)value);
}

void Widget::on_fov_slider_valueChanged(int value)
{
    Misc::changeFov(ci[0], value);
    ui->player_fov_val->setText(QString::number(value));
}

void Widget::on_glowAlpha_valueChanged(double arg1)
{
    glow_alpha = arg1;
    Glow::setGlowAlpha(glow_alpha);
}

void Widget::on_bhop__enable_stateChanged(int arg1)
{
    Misc::bhopEnabled() ? Misc::setBhopEnabled(0) : Misc::setBhopEnabled(1);
}

void Widget::on_glow_enable_stateChanged(int arg1)
{
    Glow::glowEnabled() ? Glow::setGlowEnabled(0) : Glow::setGlowEnabled(1);
}

void Widget::on_enemy_chams_stateChanged(int arg1)
{
    QColor color = QColorDialog::getColor(Qt::blue, this);
    ui->ENEMYCHAMS->setStyleSheet(QString("QPushButton{background-color: %1}").arg(color.name()));
    int arr[3] = {color.red(), color.green(), color.blue()};
    Glow::enemyChamsStateChanged(arr);
}

void Widget::on_team_chams_stateChanged(int arg1)
{
    QColor color = QColorDialog::getColor(Qt::blue, this);
    ui->TEAMCHAMS->setStyleSheet(QString("QPushButton{background-color: %1}").arg(color.name()));
    int arr[3] = {color.red(), color.green(), color.blue()};
    Glow::teamChamsStateChanged(arr);
}

void Widget::on_team_glow_stateChanged(int arg1)
{
    QColor color = QColorDialog::getColor(Qt::blue, this);
    ui->TEAMCOL->setStyleSheet(QString("QPushButton{background-color: %1}").arg(color.name()));
    Glow::setGlowTeamColor(color.red(), color.green(), color.blue(), 1);
}

void Widget::on_enemy__glow_stateChanged(int arg1)
{
    QColor color = QColorDialog::getColor(Qt::red, this);
    ui->ENEMYCOL->setStyleSheet(QString("QPushButton{background-color: %1}").arg(color.name()));
    Glow::setGlowTeamColor(color.red(), color.green(), color.blue(), 0);
}

void Widget::on_health_glow_stateChanged(int arg1)
{
    toggleHealthGlow = !toggleHealthGlow;
    Glow::setEnableHealthGlow(toggleHealthGlow);
}

void Widget::on_noflash_enable_stateChanged(int arg1)
{
    ToggleNoFlash = !ToggleNoFlash;
    Misc::setEnabledNoFlash(ToggleNoFlash);
}

void Widget::on_aim_enable_stateChanged(int arg1) {
    toggleAimbot = !toggleAimbot;
}

void Widget::on_horizontalSlider_valueChanged(int value)
{
    ui->fov_val->setText(QString::number(value));
    aimbot_fov = value;
    Aim::setFOVonSlider(aimbot_fov);

}

void Widget::on_smoth_slider_valueChanged(int value)
{
    ui->smooth_val->setText(QString::number(value));
    aimbot_smooth = value;
    Aim::setSmoothOnSlider(float(value));
}

void Widget::on_recoil_slider_valueChanged(int value)
{
    ui->recoil_val->setText(QString::number(value));
    aimbot_recoil = value;
    Aim::setRecoilControlPerc(float(value));
}

void Widget::on_aimkey_set_textChanged(const QString &arg1)
{
    aimbot_on_key = (int)(&arg1);
}

void Widget::on_silent_enable_stateChanged(int arg1)
{
    enable_silent = arg1;
    Aim::toggleSilentAim(enable_silent);
}

void Widget::on_doubleSpinBox_2_valueChanged(double arg1)
{
    nightmode_amount = arg1;
    //Misc::setNightmodeAmount(nightmode_amount);
}

void Widget::on_nightmode_enable_stateChanged(int arg1)
{
    night_state = !night_state;
    //if(night_state)
        //Misc::setNightmodeAmount(nightmode_amount);
    //else
        //Misc::setNightmodeAmount(0.f);
}



