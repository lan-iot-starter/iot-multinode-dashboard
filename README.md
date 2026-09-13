# iot-multinode-dashboard

A modular, LAN-based IoT framework featuring multi-MCU firmware (ESP32, STM32, GD2, etc.), web dashboard, mobile apps, and hardware PCB designs.

## Repository Structure

```text
iot-multinode-dashboard/
├── firmware/         # 多芯片底层固件源码
│   ├── esp32/        # ESP32 相关工程 (e.g., esp32-rgb-node)
│   └── stm32/        # STM32 相关工程
├── web/              # 前端网页端面板
├── app/              # 移动端应用 (多端适配)
├── doc/              # 项目详细开发文档
└── pcb/              # 硬件电路原理图与 PCB 设计
