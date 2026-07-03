//数据日志，黑匣子
#include "flight_logger.h"
#include <iostream>
#include <chrono>//C++标准时间库
#include <iomanip>
#include <sstream>

FlightLogger::FlightLogger() : isLogging(false) {}

FlightLogger::~FlightLogger() {
    stopLogging();
}

bool FlightLogger::startLogging() {
    // 1. 获取当前系统时间，用来做文件名，防止覆盖旧日志
    auto now = std::chrono::system_clock::now();//获取C++格式系统时间
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);//将上面时间转换成标准时间戳
    //上面两句是固定格式，即拿到系统时间
    std::stringstream ss;
    ss << "FlightLog_" << std::put_time(std::localtime(&now_c), "%Y%m%d_%H%M%S") << ".csv";
    //拼接成文件名，但此时还不是文件。


    // 2. 打开文件
    logFile.open(ss.str(), std::ios::out | std::ios::app);
    //打开日志文件，文件名是ss.str()，文件模式是std::ios::out | std::ios::app写入+追加不覆盖
    if (!logFile.is_open()) {//is_open()判断文件是否打开或新建成功
        std::cerr << "[黑匣子] 无法创建日志文件！" << std::endl;
        return false;
    }

    // 3. 写入 CSV 的表头 (列名)
    logFile << "Sequence,Roll,Pitch,Yaw,Latitude,Longitude,Altitude,Speed\n";
    //,代表切换单元格
    isLogging = true;
    std::cout << "[黑匣子] 开始记录，文件名: " << ss.str() << std::endl;
    return true;
}

void FlightLogger::stopLogging() {
    if (isLogging && logFile.is_open()) {
        logFile.close();
        isLogging = false;
        std::cout << "[黑匣子] 记录停止，文件已安全保存。" << std::endl;
    }
}

void FlightLogger::logPacket(const DataPacket& packet) {
    if (!isLogging) return;

    // 硬盘写入很慢，万一上一条还没写完，下一条又来了，文件就会写花。加锁排队
    std::lock_guard<std::mutex> lock(writeMutex);

    // 写入具体的数据，用逗号隔开
    logFile << packet.sequence << ","
            << packet.payload.roll << ","
            << packet.payload.pitch << ","
            << packet.payload.yaw << ","
            << std::fixed << std::setprecision(6) << packet.payload.latitude << ","
            << packet.payload.longitude << ","
            << std::setprecision(2) << packet.payload.altitude << ","
            << packet.payload.speed << "\n";
}