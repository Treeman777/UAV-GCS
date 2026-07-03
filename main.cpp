#include <QApplication>
#include <QMetaObject>
#include "main_window.h"
#include "udp_receiver.h"
#include "flight_logger.h"// 引入黑匣子

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;//窗口
    window.show();

    UdpReceiver receiver;//接收类
    FlightLogger logger; //日志类
    
    logger.startLogging();//启动记录日志

    receiver.onDataReceived = [&window, &logger](const DataPacket& packet) {
        //日志线程里写入数据
        logger.logPacket(packet);//写入日志数据，加了互斥锁
        
        //主线程里更新数据
        QMetaObject::invokeMethod(&window, [&window, packet]() {
            window.updateData(
                packet.payload.roll, 
                packet.payload.pitch, 
                packet.payload.yaw, 
                packet.payload.battery, 
                packet.payload.altitude, 
                packet.payload.latitude,
                packet.payload.longitude
            );
        });
    };

    //在main_window.h里声明了，然后这里用lambda定义与调用（回调）
    window.onCommandTriggered = [&receiver](CommandType cmd) {
        receiver.sendCommand(cmd);
    };

    window.onPidSync = [&receiver](float p, float i, float d) {
        receiver.sendPid(p, i, d);
    };
    
    receiver.start();//开始接收设备端发送来的数据

    int ret = app.exec();//启动QT的无限事件循环，直到关闭窗口才会继续执行

    logger.stopLogging();//关闭记录日志，保存日志内容
    
    return ret;//判断是否正常退出程序
}