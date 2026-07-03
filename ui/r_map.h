#pragma once
#include <QWidget>
#include <QWebEngineView>
#include <QVBoxLayout>

class RMap : public QWidget {
    Q_OBJECT

public:
    explicit RMap(QWidget *parent = nullptr);//构造
    ~RMap();//析构

    void updatePosition(float lat, float lon);//更新无人机

private:
    QWebEngineView *webView; // Web引擎核心
};