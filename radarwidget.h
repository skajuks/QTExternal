#ifndef RADARWIDGET_H
#define RADARWIDGET_H

#include <QWidget>
#include <QtCore>
#include <QGraphicsScene>
QT_BEGIN_NAMESPACE
namespace Ui {
class RadarWidget;
}
QT_END_NAMESPACE
class RadarWidget : public QWidget
{
    Q_OBJECT

public:
    RadarWidget(QWidget *parent = nullptr);
    ~RadarWidget();
    Ui::RadarWidget *ui;
    QGraphicsScene *scene;
    QGraphicsEllipseItem *ellipse;
    QGraphicsRectItem *rectangle;
    QGraphicsLineItem *line;
    QBrush redBrush = QBrush(Qt::red);
    QBrush blueBrush = QBrush(Qt::blue);
    QPen blackPen = QPen(Qt::black);
};

#endif // RADARWIDGET_H
