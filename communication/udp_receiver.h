#pragma once
#include <thread>
#include <atomic>
#include <functional> // 新增：引入标准回调函数库
#include "protocol.h"

class UdpReceiver {
public:
    UdpReceiver();
    ~UdpReceiver();
    
    void start(); // 启动接收引擎
    void stop();  // 停止接收引擎

    std::function<void(const DataPacket&)> onDataReceived;
    //这是一个通用函数包装器，它在main.cpp中被调用
    void sendCommand(CommandType cmd); 
    void sendPid(float p, float i, float d);

private:
    void receiveLoop(); //这是一个函数
    
    int sockfd;
    std::atomic<bool> is_running; //原子类型定义的布尔量，控制线程之间交互
    std::thread work_thread;      //新建一个专门用来接收数据包的线程
};