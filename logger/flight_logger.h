#pragma once
#include <string>
#include <fstream>//文件流头文件
#include <mutex>//互斥锁头文件
#include "protocol.h"

//飞行日志
class FlightLogger {
public:
    FlightLogger();//构造
    ~FlightLogger();//析构

    // 启动记录，自动生成带时间戳的文件名
    bool startLogging(); 
    // 停止记录，安全保存文件
    void stopLogging();  
    // 写入一行数据 (线程安全)
    void logPacket(const DataPacket& packet); 

private:
    std::ofstream logFile; // C++ 标准文件输出流
    std::mutex writeMutex; // 互斥锁：防止多线程同时写文件导致内容错乱
    bool isLogging;
};