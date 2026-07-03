#pragma once//防止头文件重复编译

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStatusBar>    // 状态栏
#include <QProgressBar>  // 进度条
#include <QTimer>        // 定时器
#include <QLabel>        // 标签栏
#include <QPushButton> // 按钮
#include <functional>  // 回调函数

#include "protocol.h"//协议包
#include "attitude_indicator.h"//姿态仪
#include "r_map.h"//实时地图
#include "pid_panel.h"//PID修改界面

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);//指定mainwindow没有父窗口
    void updateData(float roll, float pitch, float yaw, float battery, float altitude, float lat, float lon); 
    //roll是翻滚角，pitch是俯仰角，yaw是航向角，battery是电池，altitude是飞行高度，lat是纬度，lon是经度
    
    //触发器函数
    std::function<void(CommandType)> onCommandTriggered;
    std::function<void(float, float, float)> onPidSync;

private:
    AttitudeIndicator* attitudeWidget;//姿态仪的对象
    RMap* rmapWidget; //实时地图的对象
    PidPanel* pidPanel; //PID输入框的对象
    
    // 底部状态栏零件
    QStatusBar* statusBar;
    QProgressBar* batteryBar;
    QLabel* connectionLabel;
    QTimer* watchdogTimer; 

    // 两个实体按钮
    QPushButton* btnTakeoff;//起飞按钮
    QPushButton* btnLand;//降落按钮
    QLabel* altitudeLabel; // 【新增】高度表指示器
};