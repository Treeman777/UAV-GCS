#pragma once
#include <QWidget>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <functional>

class PidPanel : public QWidget {
    Q_OBJECT
public:
    explicit PidPanel(QWidget *parent = nullptr);
    //Qt窗口类必须要加的构造函数，可当有参构造用，也能当无参构造用。如果删掉这句，就无法调用QWidget的构造，Qt类窗口直接失效
    
    // 信号插座：当按下同步时，把 P, I, D 传出去
    std::function<void(float, float, float)> onPidSyncTriggered;//回调函数

private:
    QDoubleSpinBox *pSpin, *iSpin, *dSpin;
    QPushButton *syncBtn;
};