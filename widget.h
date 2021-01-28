#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    Ui::Widget *ui;
    static bool setBhopEnable();

private slots:
    void on_bhop__enable_stateChanged(int arg1);
    void on_glow_enable_stateChanged(int arg1);
    void on_team_glow_stateChanged(int arg1);

    void on_enemy__glow_stateChanged(int arg1);

    void on_health_glow_stateChanged(int arg1);

    void on_noflash_enable_stateChanged(int arg1);

    void on_aim_enable_stateChanged(int arg1);

    void on_horizontalSlider_valueChanged(int value);

    void on_smoth_slider_valueChanged(int value);

    void on_recoil_slider_valueChanged(int value);

    void on_aimkey_set_textChanged(const QString &arg1);

    void on_silent_enable_stateChanged(int arg1);

    void on_doubleSpinBox_2_valueChanged(double arg1);

    void on_nightmode_enable_stateChanged(int arg1);

    void on_glowAlpha_valueChanged(double arg1);

    void on_Flashvalue_valueChanged(double arg1);

private:

};
#endif // WIDGET_H
