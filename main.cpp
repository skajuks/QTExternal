#include "widget.h"
#include "Misc.h"
#include <iostream>
#include <limits>
#include <cstddef>
#include "Functions.h"
#include "aimbot.h"
#include <QApplication>
#include <QMessageBox>
#include <QColorDialog>
#include <thread>                   // fix these damn imports so they are grouped by libs and actual feature headers
#include <iostream>
#include "paint.h"
#include "fakelag.h"
#include "offsets.hpp"
#include "Structs.h"
#include "entity.h"
#include "esp.h"
#include <QStyleFactory>
#include "stylesheet.h"
#include <QTimer>

#include "widget.h"
#include "ui_widget.h"

#undef max // C macro for max variable from math.h - have to undef because class::max() would paste macro content at "max" (class:((a)>(b)?(a):(b)))
#define LEFT_MOUSE_BUTTON 0x01

ClientInfo ci[64];  // ci[0] = localplayer
Entity e[64];       // e[0] = localplayer

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
bool  jump_shot         = false;    // ADD AIMBOT TOGGLE ON AIMKEY LMAO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
bool  toggle_aimbot_key = false;

extern int aimfov;

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

class Glow;

int main(int argc, char** argv) {
    auto app = new QApplication(argc, argv);
    auto window = new Widget;

    qApp->setStyle(QStyleFactory::create("Fusion"));
    qApp->setStyleSheet(style_sheet);

    window->show();

    //AllocConsole();
    //freopen("CONOUT$", "w", stdout);

    // add a sigscan here

    uintptr_t glowObjectManager = Memory.readMem<uintptr_t>(gameModule + dwGlowObjectManager);

    // here goes static features that doesn't need updates

    Glow::setBrightness();
    //Misc::setNightmodeAmount(0.09f);


    QTimer *timer = new QTimer(window);
    QObject::connect(timer, &QTimer::timeout, [&](){

        int entityIndex = 1;
        float oldx = std::numeric_limits<float>::max();
        float oldy = std::numeric_limits<float>::max();
        int targetIndex = NULL;
        int aliveEntity = 0;
        aimbotVariables variables;

        // Under here goes everything that needs to be updated for every entity except local player

        Memory.readMemTo<ClientInfo>(gameModule + dwLocalPlayer, &ci[0]);
        Memory.readMemTo<Entity>(ci[0].entity, &e[0]);

        do {

            if(entityIndex >= 64) break;

            Memory.readMemTo<ClientInfo>(gameModule + dwEntityList + entityIndex * 0x10, &ci[entityIndex]);
            Memory.readMemTo<Entity>(ci[entityIndex].entity, &e[entityIndex]);

            if(e[entityIndex].health > 0 && ci[entityIndex].entity) { // checks if entity exists and is alive

                aliveEntity++;

                if(e[entityIndex].team == e[0].team){
                    Glow::ProcessEntityTeam(ci[entityIndex], glowObjectManager);
                }
                else {
                    Glow::ProcessEntityEnemy(ci[entityIndex], e[entityIndex], glowObjectManager);
                    //if(!toggleAimbot){
                        VECTOR2 newDistance = Aim::getClosestEntity(e[0], ci[entityIndex]);
                        if(newDistance.x < oldx && newDistance.y < oldy && newDistance.x <= aimfov && newDistance.y <= aimfov){
                            oldx = newDistance.x;
                            oldy = newDistance.y;
                            targetIndex = entityIndex;   // execute code only if aimbot is actually enabled
                        }
                    //}
                }
            }
        } while(ci[entityIndex++].nextEntity);

        // Update glow for target entity

        Glow::ProcessTargetEntity(ci[targetIndex], glowObjectManager);

        // Under here goes everything that needs to be updated for local player only!

        Misc::setBhop(e[0]);
        Misc::setNoFlash(ci[0]);
        INPUT inp[1] = {0};
        if(GetAsyncKeyState(0x56) && toggleAimbot && targetIndex && jump_shot) {
            if(e[0].vecVelocity.z < 15 && e[0].vecVelocity.z > 0){
                inp[0].type = INPUT_MOUSE;
                inp[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                SendInput(1, inp, sizeof(INPUT));
            }
        }
        if(GetAsyncKeyState(LEFT_MOUSE_BUTTON) && toggleAimbot && targetIndex) {
            if(jump_shot && e[0].vecVelocity.z < 15 && e[0].vecVelocity.z > 0){
                inp[0].type = INPUT_MOUSE;
                inp[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
                SendInput(1, inp, sizeof(INPUT));
            }
            if(toggle_aimbot_key) {
                if (GetAsyncKeyState(0x43)){
                    variables = Aim::executeAimbot(ci[targetIndex], e[0], ci[0]);
                }
            } else {
                variables = Aim::executeAimbot(ci[targetIndex], e[0], ci[0]);        // execute code only if aimbot is actually enabled
            }
        }
        // debug section

        if(toggleAimbot){
            if(toggle_aimbot_key){
                 window->ui->aimbot_state_ui->setText(QString("AIMBOT ACTIVE [ ARMED ON KEY ]"));
            } else {
                 window->ui->aimbot_state_ui->setText(QString("AIMBOT ACTIVE"));
            }
            window->ui->aimbot_state_ui->setStyleSheet(QString("QLabel{color: rgba(0,255,0,255)}"));
        } else {
            window->ui->aimbot_state_ui->setText(QString("AIMBOT ON STANDBY"));
            window->ui->aimbot_state_ui->setStyleSheet(QString("QLabel{color: rgba(255,255,0,255)}"));
        }

        float velocity = e[0].vecVelocity.z;
        window->ui->loc_player_vel->setText(QString::number(velocity));
        velocity > 0 ? window->ui->loc_player_vel->setStyleSheet(QString("QLabel{color: rgba(0,255,0,255)}")) : window->ui->loc_player_vel->setStyleSheet(QString("QLabel{color: rgba(255,255,0,255)}"));
        window->ui->curr_ent_cnt_val->setText(QString::number(aliveEntity));
        window->ui->loc_p_ent_num->setText(QString::number(ci[0].entity));
        window->ui->curr_tgt->setText(QString::number(targetIndex));
        window->ui->dist_x->setText(QString::number(oldx));
        window->ui->dist_y->setText(QString::number(oldy));
        window->ui->ent_recoil->setText(QString(QString::number(variables.recoil.x) + " : " + QString::number(variables.recoil.y) + " : " +
                                                QString::number(variables.recoil.z)));

        window->ui->ent_weapon->setText(QString::number(variables.entityWeapon));
        window->ui->ent_angles->setText(QString(QString::number(variables.AimAngle.x) + " : " + QString::number(variables.AimAngle.y) + " : " +
                                                QString::number(variables.AimAngle.z)));

    });
    timer->start(1);

    std::thread(ESP::run).detach();     // d9x9 thread

    return app->exec();
}

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
    Glow::StateChanged(1); // esp master
}

void Widget::on_fakelag_slider_valueChanged(int value)
{
    FakeLag::setFakelag(value);
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
    //Misc::changeFov(value);
    ui->player_fov_val->setText(QString::number(value));
}

void Widget::on_Flashvalue_valueChanged(double arg1)
{
    flashAmount = (int)arg1;
}

void Widget::on_glowAlpha_valueChanged(double arg1)
{
    glow_alpha = arg1;
    Glow::setGlowAlpha(glow_alpha);
}

void Widget::on_bhop__enable_stateChanged(int arg1)
{
    if (Misc::bhopEnabled())
        Misc::setBhopEnabled(0);
    else
        Misc::setBhopEnabled(1);
}

void Widget::on_glow_enable_stateChanged(int arg1)
{
    if (Glow::glowEnabled())
        Glow::setGlowEnabled(0);
    else
        Glow::setGlowEnabled(1);
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

void Widget::on_aim_enable_stateChanged(int arg1) { toggleAimbot = !toggleAimbot; }

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



