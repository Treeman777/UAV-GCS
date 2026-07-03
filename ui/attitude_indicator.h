#pragma once
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>

class AttitudeIndicator : public QWidget {
    Q_OBJECT

public:
    explicit AttitudeIndicator(QWidget *parent = nullptr);//构造函数
    
    void updateAttitude(float roll, float pitch, float yaw);//实时更新三维轴向角

protected:
    void paintEvent(QPaintEvent *event) override;//QPaintEvent函数重写

private:
    float m_roll = 0.0f;
    float m_pitch = 0.0f;
    float m_yaw = 0.0f;
};