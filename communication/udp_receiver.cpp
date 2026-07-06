#include "udp_receiver.h"
#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>  // 【核心修复：加上这把翻译 IP 地址的专用扳手！】

//CRC算法
//将核心数据传入校验
uint16_t crc16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            if (crc & 1) 
                crc = (crc >> 1) ^ 0xA001;
            else 
                crc >>= 1;
        }
    }
    return crc;
}

//构造函数初始化赋值sockfd为-1，is_running为false
UdpReceiver::UdpReceiver() : sockfd(-1), is_running(false) {}
//析构函数执行stop()，即停止接收设备端信号
UdpReceiver::~UdpReceiver() { 
    stop(); 
}

void UdpReceiver::start() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) //如果创建套接字失败，就退出
        return;

    sockaddr_in local_addr;//sockaddr_in是IPv4地址结构体
    memset(&local_addr, 0, sizeof(local_addr));//结构体初始化
    local_addr.sin_family = AF_INET;//用什么互联网套接字协议族family
    local_addr.sin_addr.s_addr = INADDR_ANY; //在本机哪个IP地址上监听
    local_addr.sin_port = htons(8080);//端口号  

    if (bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        std::cerr << "端口绑定失败！" << std::endl;//标准错误输出流
        return;
    }

    is_running = true;//原子布尔类型开始运行
    work_thread = std::thread(&UdpReceiver::receiveLoop, this);
    //这个线程开始执行receiveloop函数
    std::cout << "防空雷达已启动，开启强制 CRC 校验" << std::endl;
}

void UdpReceiver::stop() {
    is_running = false;//原子布尔类型关闭
    if (sockfd >= 0) { 
        close(sockfd); 
        sockfd = -1; 
    }//关闭套接字
    if (work_thread.joinable()) { 
        work_thread.join(); 
    }
    //判断线程是否还在运行，未被回收。若未被回收，则等待其安全结束，再回收资源
}

void UdpReceiver::receiveLoop() {
    DataPacket packet;//定义一个数据包
    while (is_running) {
        ssize_t bytes_read = recvfrom(sockfd, reinterpret_cast<char*>(&packet), sizeof(packet), 0, nullptr, nullptr);
        
        if (bytes_read == sizeof(DataPacket)) {//长度是否一致
            // 1. 验证帧头帧尾
            if (packet.header == 0xAA55 && packet.tail == 0x55AA) {
                // 必须严格对应发送端的校验范围：长度(2) + 序号(4) + 载荷(40)
                size_t checksum_length = sizeof(packet.length) + sizeof(packet.sequence) + sizeof(packet.payload);
                const uint8_t* checksum_start = reinterpret_cast<const uint8_t*>(&packet.length);
                uint16_t computed_crc = crc16(checksum_start, checksum_length);

                // 2. 将自己算出来的 CRC 和包里携带的 CRC 进行对比！
                if (computed_crc == packet.crc16) {
                    if (onDataReceived) {
                        //这里的onDataReceived也是udp.h文件里的函数
                        onDataReceived(packet); 
                    }
                } else {
                    std::cerr << "[警报] 发现损坏包！Seq: " << packet.sequence 
                              << " | 期望 CRC: " << computed_crc 
                              << " | 实际 CRC: " << packet.crc16 << std::endl;
                }
            }
        }
    }
}

void UdpReceiver::sendCommand(CommandType cmd) {
    if (sockfd < 0) return;

    CommandPacket packet;
    packet.command = cmd;
    packet.param1 = (cmd == CommandType::TAKEOFF) ? 100.0f : 0.0f; // 起飞设定到 100 米，降落设定为 0 米

    // 计算上行链路的 CRC 校验
    size_t checksum_length = sizeof(packet.command) + sizeof(packet.param1) + sizeof(packet.param2);
    const uint8_t* checksum_start = reinterpret_cast<const uint8_t*>(&packet.command);
    packet.crc16 = crc16(checksum_start, checksum_length);

    // 瞄准无人机的命令接收端口：8081
    sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(8081); 
    inet_pton(AF_INET, "127.0.0.1", &target_addr.sin_addr);
    //将127.0.0.1这个字符串地址转换成二进制字节格式的地址

    sendto(sockfd, reinterpret_cast<const char*>(&packet), sizeof(packet), 0,
           (const sockaddr*)&target_addr, sizeof(target_addr));
           
    std::cout << "[上行链路] 指令已发射，代码: " << (int)cmd << std::endl;
}

void UdpReceiver::sendPid(float p, float i, float d) {
    if (sockfd < 0) return;

    PidPacket packet;
    packet.p = p;
    packet.i = i;
    packet.d = d;

    // CRC 校验
    size_t checksum_length = sizeof(packet.p) + sizeof(packet.i) + sizeof(packet.d);
    const uint8_t* checksum_start = reinterpret_cast<const uint8_t*>(&packet.p);
    packet.crc16 = crc16(checksum_start, checksum_length);

    sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(8081); // 同样发给 8081 端口
    inet_pton(AF_INET, "127.0.0.1", &target_addr.sin_addr);

    sendto(sockfd, reinterpret_cast<const char*>(&packet), sizeof(packet), 0,
           (const sockaddr*)&target_addr, sizeof(target_addr));
    
    std::cout << "[上行链路] PID 参数已更新 -> P:" << p << " I:" << i << " D:" << d << std::endl;
}