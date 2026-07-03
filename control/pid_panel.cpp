//PID的显示框
#include "pid_panel.h"

PidPanel::PidPanel(QWidget *parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    
    // 设置标题
    auto* title = new QLabel("PID 调参", this);
    title->setStyleSheet("color: yellow; font-weight: bold; font-size: 16px;");
    layout->addWidget(title);

    // 辅助函数：快速创建带标签的输入框
    auto createInput = [&](QString name, double initVal) {
        layout->addWidget(new QLabel(name + ":", this));
        auto* sb = new QDoubleSpinBox(this);
        sb->setRange(0, 10.0);
        sb->setSingleStep(0.1);//步长
        sb->setValue(initVal);
        layout->addWidget(sb);
        return sb;
    };

    pSpin = createInput("比例 (P)", 1.0);
    iSpin = createInput("积分 (I)", 0.0);
    dSpin = createInput("微分 (D)", 0.0);

    syncBtn = new QPushButton("一键同步参数", this);
    syncBtn->setStyleSheet("background-color: #ff8c00; color: black; font-weight: bold; height: 40px; margin-top: 10px;");
    layout->addWidget(syncBtn);

    connect(syncBtn, &QPushButton::clicked, this, [this](){
        if(onPidSyncTriggered) onPidSyncTriggered(pSpin->value(), iSpin->value(), dSpin->value());
    });//这是输入的第一关，把PID输入之后，同步到Pid触发器上，再由触发器传给待定同步区，
    // 最后将待定同步区的PID内容通过sendPid发送给设备端
}

//syncBtn->onPidSyncTriggered（PID）->onPidSync（mainwindow）->sendPid（main）