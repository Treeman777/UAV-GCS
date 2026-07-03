# GCS Ground Control Station (Qt + UDP + Mock)

基于 Qt + C++ 开发的无人机地面站系统，包含实时数据通信、姿态显示、PID控制面板以及仿真模块（Mock），用于模拟无人机飞行数据并与地面站进行通信。

---

## 项目功能

- 基于 UDP 的实时数据通信
- PID 控制参数调节
- Qt 可视化地面站界面
- 姿态显示模块
- 地图与数据可视化
- Mock 仿真数据源
- 飞行数据日志记录
- CMake 工程管理

---

## 系统结构

Mock 仿真模块通过 UDP 发送数据到地面站，地面站接收并解析数据后进行显示与控制。

Mock → UDP → GCS地面站（Qt界面 + 数据处理 + 控制）

---

## 模块划分

ui              Qt界面相关（主窗口、姿态仪表、PID面板）
communication   UDP通信和协议解析
control         PID控制算法
logger          日志记录
simulation      Mock仿真数据生成

---

## 技术栈

C++
Qt6 (Widgets, Network, WebEngineWidgets)
UDP Socket
CMake
多进程架构

---

## 编译方法

mkdir build
cd build
cmake ..
make

---

## 运行方法

先启动仿真模块：

./mock

再启动地面站：

./gcs_ui

或者使用一键启动：

bash run.sh

---

## run.sh 示例

#!/bin/bash
./mock &
sleep 1
./gcs_ui

---

## 项目结构

GCS/
build/
ui/
communication/
control/
logger/
simulation/
main.cpp
CMakeLists.txt
run.sh
README.md

---

## 实现说明

通信模块使用UDP进行数据传输，Mock模块模拟飞行器发送姿态数据，地面站解析后进行显示。

控制模块实现PID控制逻辑。

UI模块基于Qt Widgets实现界面显示与交互。

Mock模块用于生成测试数据。

---

##fenglinchenxi

C++ Qt 学习项目，用于地面站与无人机通信方向实践