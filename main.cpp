#include "widget.h"
#include "Misc.h"
#include "Visuals.h"
#include "noflash.h"
#include "Functions.h"
#include "aimbot.h"
#include "Memory.hpp"
#include <QApplication>
#include <QMessageBox>
#include <QColorDialog>
#include <thread>
#include <iostream>
#include "Esp.h"
#include "fakelag.h"

#include "widget.h"
#include "ui_widget.h"

bool toggleHealthGlow = false;
bool ToggleNoFlash = false;
bool toggleAimbot = false;
int aimbot_fov = 1;
bool night_state = false;
float aimbot_smooth = 1.f;
float aimbot_recoil = 1.f;
float glow_alpha = 1.f;
int flashAmount = 0;
float nightmode_amount = 0.04f;
int aimbot_on_key = 0x01; // mouse 1 default
bool enable_silent = false;

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
    Misc::changeFov(value);
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
    if (_Bhop::bhopEnabled())
        _Bhop::setBhopEnabled(0);
    else
        _Bhop::setBhopEnabled(1);
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
    Flash::setEnabledNoFlash(ToggleNoFlash);
}

void Widget::on_aim_enable_stateChanged(int arg1)
{
    toggleAimbot = !toggleAimbot;
    Aim::toggleAimbotOnKey(toggleAimbot);
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
    Flash::setNightmodeAmount(nightmode_amount);
}

void Widget::on_nightmode_enable_stateChanged(int arg1)
{
    night_state = !night_state;
    if(night_state)
        Flash::setNightmodeAmount(nightmode_amount);
    else
        Flash::setNightmodeAmount(0.f);
}


int main(int argc, char** argv)
{
    auto app = new QApplication(argc, argv);
    auto window = new Widget;
    window->show();
    window->ui->aimBonesList->addItem("Head - 8");

    std::thread (_Bhop::initialize).detach();
    std::thread (Glow::ESP).detach();
    std::thread (Flash::noflash).detach();
    std::thread (Aim::AIMBOT).detach(); 
    std::thread (ESP::run).detach();
    //Esp();
    //std::thread (ESP::run).detach();
    //std::thread ([](){Esp();}).detach();
    //std::thread (FakeLag::fakeLag).detach();

    return app->exec();
}

