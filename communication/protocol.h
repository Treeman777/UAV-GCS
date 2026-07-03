//协议包
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>

// 禁用内存对齐
#pragma pack(push, 1)

//核心遥测数据 - 40字节
struct Data {
    double latitude;  // 纬度
    double longitude; // 经度
    float altitude;   // 高度
    float speed;      // 速度
    float pitch;      // 俯仰角
    float roll;       // 翻滚角
    float yaw;        // 航向角
    float battery;    // 电池电压
};

// 完整数据包 - 52字节
struct DataPacket {
    uint16_t header = 0xAA55;      // 帧头
    uint16_t length = sizeof(Data); // 载荷长度
    uint32_t sequence = 0;         // 帧序号
    Data payload;         // 载荷数据
    uint16_t crc16 = 0;            // CRC16校验
    uint16_t tail = 0x55AA;        // 帧尾
};


// 地面站发给无人机的命令包（上行控制指令）
// 发送指令
enum class CommandType : uint8_t {
    TAKEOFF = 0x01,   // 一键起飞
    LAND = 0x02,      // 一键降落
    //RETURN = 0x03,    // 一键返航
    //HOVER = 0x04      // 紧急悬停
};

// 上行指令数据包 - 13字节
struct CommandPacket {
    uint16_t header = 0xBB66;        // 上行帧头用 BB66,2字节
    CommandType command;             // 1 字节：动作指令
    float param1 = 0.0f;             // 4 字节：参数1 (比如起飞目标高度)
    float param2 = 0.0f;             // 4 字节：参数2
    uint16_t crc16 = 0;              // 2 字节：命令校验和，绝不能让无人机执行乱码！
};

//PID数据包 - 16字节
struct PidPacket {
    uint16_t header = 0xCC77; // 上行调参PID帧头2字节
    float p;                  // 4 字节
    float i;                  // 4 字节
    float d;                  // 4 字节
    uint16_t crc16 = 0;       // 2 字节
};

#pragma pack(pop)

#endif