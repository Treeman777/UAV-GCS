#include "attitude_indicator.h"
#include <QPainterPath>

AttitudeIndicator::AttitudeIndicator(QWidget *parent) : QWidget(parent) {
    setMinimumSize(300, 300);
}

void AttitudeIndicator::updateAttitude(float roll, float pitch, float yaw) {
    m_roll = roll;
    m_pitch = pitch;
    m_yaw = yaw;
    
    update(); 
}

void AttitudeIndicator::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);//告诉编译器，这个形参暂时不用，别报变量未使用的警告
    
    QPainter painter(this);//创建一支画笔，绑定到当前窗口，准备开始画画
    
    // 开启抗锯齿
    painter.setRenderHint(QPainter::Antialiasing);

    int width = this->width();
    int height = this->height();
    int centerX = width / 2;
    int centerY = height / 2;

    //填充矩阵，用深灰色填充背景
    painter.fillRect(0, 0, width, height, QColor(30, 30, 30));
    
    //平移绘图坐标系原点，窗口中心点变成坐标原点
    painter.translate(centerX, centerY);
    
    // 根据翻滚角(Roll)旋转整个画板，负号是指让画面对应无人机的反向旋转，达到仿真效果
    painter.rotate(-m_roll); 
    
    // 根据俯仰角(Pitch)上下平移画板 (假设 1度 = 3像素)，x轴不左右移动，y轴上下偏移
    painter.translate(0, m_pitch * 3.0); 
    
    // 天空
    painter.setBrush(QColor(50, 150, 255));
    painter.drawRect(-width, -height*2, width*2, height*2);

    // 土地
    painter.setBrush(QColor(160, 80, 50));
    painter.drawRect(-width, 0, width*2, height*2);

    // 地平线
    painter.setPen(QPen(Qt::white, 3));
    painter.drawLine(-width, 0, width, 0);

    painter.resetTransform(); //重置
    painter.translate(centerX, centerY); //原点放回中心

    // 无人机
    painter.setPen(QPen(Qt::red, 4));
    painter.drawLine(-30, 0, -10, 0); // 左翼
    painter.drawLine(10, 0, 30, 0);   // 右翼
    painter.drawLine(0, -10, 0, 10);  // 垂直尾翼
    
    //实时数据
    painter.resetTransform();
    painter.setPen(QPen(Qt::green, 1));
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(10, 20, QString("R: %1").arg(m_roll, 0, 'f', 1));
    painter.drawText(10, 40, QString("P: %1").arg(m_pitch, 0, 'f', 1));
    painter.drawText(10, 60, QString("Y: %1").arg(m_yaw, 0, 'f', 1));
}