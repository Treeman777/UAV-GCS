#include "main_window.h"
#include "r_map.h"

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
    setWindowTitle("GCS - 无人机终端");//设置窗口名称
    resize(800, 450); //定义窗口大小

    this->setStyleSheet(R"(
        QWidget {
            background-color: #0A0C10; 
            color: #00FFCC; 
            font-family: "Courier New", monospace; 
        }
        
        QLabel {
            font-size: 15px;
            font-weight: bold;
        }
        
        QPushButton {
            background-color: #161A22;
            border: 2px solid #00FFCC;
            border-radius: 4px;
            color: #00FFCC;
            padding: 5px;
            font-weight: bold;
        }
    
        QPushButton:hover {
            background-color: #00FFCC;
            color: #0A0C10;
        }
        QPushButton:pressed {
            background-color: #00CCAA;
        }
        
        QDoubleSpinBox {
            background-color: #161A22;
            border: 1px solid #444;
            border-radius: 3px;
            color: #FFF;
            padding: 4px;
        }
        QDoubleSpinBox:focus {
            border: 1px solid #00FFCC;
        }
        
        QStatusBar {
            background-color: #000000;
            border-top: 1px solid #333;
        }

        QProgressBar {
            background-color: #161A22;
            border: 1px solid #555;
            border-radius: 2px;
            text-align: center;
            color: white;
            font-weight: bold;
        }

        QProgressBar::chunk {
            background-color: #00FFCC;
        }
    )");
    // ----------------------

    attitudeWidget = new AttitudeIndicator(this);//姿态仪
    rmapWidget = new RMap(this); //实时地图
    pidPanel = new PidPanel(this);//PID输入框

    //用于同步PID信号，将上位机的PID信号传给设备端
    pidPanel->onPidSyncTriggered = [this](float p, float i, float d){
        if(onPidSync) onPidSync(p, i, d);
    };

    altitudeLabel = new QLabel("当前高度: 0.0 m", this);//飞行高度显示
    altitudeLabel->setStyleSheet("font-size: 28px; color: cyan; font-weight: bold; background-color: #111; padding: 10px; border: 2px solid cyan; border-radius: 5px;");
    altitudeLabel->setAlignment(Qt::AlignCenter);

    btnTakeoff = new QPushButton("一键起飞 (TAKEOFF)", this);//起飞按钮
    btnTakeoff->setStyleSheet("background-color: darkred; color: white; font-weight: bold; height: 40px;");
    
    btnLand = new QPushButton("一键降落 (LAND)", this);//降落按钮
    btnLand->setStyleSheet("background-color: darkblue; color: white; font-weight: bold; height: 40px;");

    connect(btnTakeoff, &QPushButton::clicked, this, [this](){
        if(onCommandTriggered) onCommandTriggered(CommandType::TAKEOFF);
    });
    connect(btnLand, &QPushButton::clicked, this, [this](){
        if(onCommandTriggered) onCommandTriggered(CommandType::LAND);
    });

    //右侧界面
    QVBoxLayout* rightLayout = new QVBoxLayout();//垂直布局
    rightLayout->addWidget(rmapWidget);
    rightLayout->addWidget(altitudeLabel); 
    rightLayout->addWidget(btnTakeoff);
    rightLayout->addWidget(btnLand);

    //左侧界面
    QVBoxLayout* leftLayout = new QVBoxLayout();
    leftLayout->addWidget(attitudeWidget, 2); // 占 2 份比例
    leftLayout->addWidget(pidPanel, 1);        // 占 1 份比例
    //整体界面比例
    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->addLayout(leftLayout, 1);
    topLayout->addLayout(rightLayout, 1);

    statusBar = new QStatusBar(this);//创建底部状态栏
    connectionLabel = new QLabel("状态: 等待连接", this);
    batteryBar = new QProgressBar(this);
    batteryBar->setRange(0, 100);
    batteryBar->setMaximumWidth(150);
    statusBar->addWidget(connectionLabel);//标签放在状态栏里面
    statusBar->addPermanentWidget(new QLabel("电池: ", this));//永久显示标签
    statusBar->addPermanentWidget(batteryBar);//永久显示电池进度条

    QVBoxLayout* mainLayout = new QVBoxLayout(this);//整体布局实现
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(statusBar);

    watchdogTimer = new QTimer(this);
    connect(watchdogTimer, &QTimer::timeout, this, [this](){
        connectionLabel->setText("状态: 链路中断！");//statusBar那里的，显示连接状态
        connectionLabel->setStyleSheet("color: red; font-weight: bold;");
    });
    watchdogTimer->start(2000); //两秒连接不上判定超时
}

void MainWindow::updateData(float roll, float pitch, float yaw, float battery, float altitude, float lat, float lon) {
    watchdogTimer->start(2000); 
    connectionLabel->setText("状态: 已连接 (50Hz)");//状态更改
    connectionLabel->setStyleSheet("color: green;");
    int batteryPercent = (battery / 25.2f) * 100; //电池电量是25.2f
    batteryBar->setValue(batteryPercent);

    altitudeLabel->setText(QString("当前高度: %1 m").arg(altitude, 0, 'f', 1));
    attitudeWidget->updateAttitude(roll, pitch, yaw);
    rmapWidget->updatePosition(lat, lon);
}