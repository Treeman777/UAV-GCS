#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <cmath>
#include <fcntl.h> // 用于设置非阻塞模式
#include "protocol.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

//计算crc发送侧函数，需要的形参是需要发送的数据以及数据长度
uint16_t calculate_crc16(const uint8_t* data, size_t length) {
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

int main() {
    std::cout << "--- 虚拟无人机设备端 ---" << std::endl;

    //1.发射遥测数据的管子(向8080发)
    int send_fd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in target_addr;
    std::memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &target_addr.sin_addr);
    //AF_INET是IPv4，3号位是存储结果的网络地址结构体
    //把该IP地址转化成二进制格式

    //2.接收控制指令的管子(监听8081)
    int recv_fd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in recv_addr;
    std::memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(8081);
    recv_addr.sin_addr.s_addr = INADDR_ANY;
    bind(recv_fd, (struct sockaddr*)&recv_addr, sizeof(recv_addr));//绑定套接字和IP地址
    fcntl(recv_fd, F_SETFL, O_NONBLOCK); //文件控制函数，把套接字设置成非阻塞模式
    //为什么要设置非阻塞模式？
    
    DataPacket packet;//数据包是在这声明的
    double t = 0.0;
    
    float current_altitude = 0.0f;
    float target_altitude = 0.0f;

    float p_gain = 1.0f, i_gain = 0.0f, d_gain = 0.0f;
    float integral = 0.0f;     // 误差累加器
    float prev_error = 0.0f;   // 上一次的误差
    float velocity = 0.0f;     // 飞机的当前爬升速度

    while (true) {
        CommandPacket cmd_pkt;
        //收到了地面站发来的消息
        if (recvfrom(recv_fd, reinterpret_cast<char*>(&cmd_pkt), sizeof(cmd_pkt), 0, nullptr, nullptr) > 0) {
            if (cmd_pkt.header == 0xBB66) {
                //帧头对了就没错了，可以继续执行
                if (cmd_pkt.command == CommandType::TAKEOFF) {
                    target_altitude = cmd_pkt.param1; // 起飞到 100 米
                    //这是由另一端的信号发送过来改变的
                    std::cout << "\n>>> [无人机] 收到起飞指令！开始爬升至: " << target_altitude << "m <<<\n" << std::endl;
                } else if (cmd_pkt.command == CommandType::LAND) {
                    target_altitude = cmd_pkt.param1; // 降落到 0 米
                    std::cout << "\n>>> [无人机] 收到降落指令！开始下降！ <<<\n" << std::endl;
                }else if (*(uint16_t*)&cmd_pkt == 0xCC77) {//简单判断是不是调参包
                    //这里是不是写错了？cmd_pkt的地址为0xCC77不应该跟0xBB66并列吗？
                    PidPacket* pid_pkt = (PidPacket*)&cmd_pkt;
                    // 【新增：把地面站发来的参数，覆盖掉旧参数】
                    p_gain = pid_pkt->p;
                    i_gain = pid_pkt->i;
                    d_gain = pid_pkt->d;
                    integral = 0.0f; // 参数变了，清空历史累加积分，防止炸机
                    std::cout << "\n>>> [无人机] 飞行参数已重写！P:" << p_gain << " I:" << i_gain << " D:" << d_gain << " <<<\n" << std::endl;
                }
            }
        }

        // --- PID算法 ---
        float error = target_altitude - current_altitude;
        
        integral += error * 0.02f; //循环周期
        if(integral > 50.0f) integral = 50.0f;
        if(integral < -50.0f) integral = -50.0f;

        float derivative = (error - prev_error) / 0.02f;
        prev_error = error;

        float thrust = (p_gain * error) + (i_gain * integral) + (d_gain * derivative);

        velocity += thrust * 0.02f;
        velocity *= 0.95f; 
        current_altitude += velocity * 0.02f;

        if (current_altitude <= 0.0f) {
            current_altitude = 0.0f;
            velocity = 0.0f;
        }
        // ------------------------------------

        //--- 打包下行数据 ---
        packet.payload.altitude = current_altitude;
        packet.payload.pitch = 20.0f * sin(t);    
        packet.payload.roll  = 30.0f * cos(t);    
        packet.payload.yaw   = fmod(t * 57.29, 360.0);

        //0.0001度大概相当于10米
        packet.payload.latitude = 31.2304 + 0.0005 * sin(t * 0.5); 
        packet.payload.longitude = 121.4737 + 0.0005 * cos(t * 0.5);

        packet.payload.battery = 25.2f - (current_altitude / 100.0f) * 1.5f; // 爬得越高越耗电
        packet.sequence++;

        size_t checksum_length = sizeof(packet.length) + sizeof(packet.sequence) + sizeof(packet.payload);
        const uint8_t* checksum_start = reinterpret_cast<const uint8_t*>(&packet.length);
        packet.crc16 = calculate_crc16(checksum_start, checksum_length);
        //这句就是计算crc16的号码，然后赋值到packet里面的crc16里面

        sendto(send_fd, reinterpret_cast<const char*>(&packet), sizeof(packet), 0, (const sockaddr*)&target_addr, sizeof(target_addr));

        t += 0.05;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    close(send_fd);
    close(recv_fd);
    return 0;
}