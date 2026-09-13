# ESP32 Firmware Nodes (`/firmware/esp32`)

This directory contains ESP32-based hardware node implementations for the `iot-multinode-dashboard` project. 

## 📂 Sub-Projects

| Project Folder | Description | Target Hardware |
| :--- | :--- | :--- |
| [`esp32-rgb-node`](./esp32-rgb-node) | RGB LED 状态控制与传感器节点 | ESP32 / ESP32-C3 / ESP32-S3 |

## 🛠️ Development Environment & Toolchain
- **Framework**: ESP-IDF / Arduino IDE / PlatformIO
- **Protocol**: Wi-Fi / Local Area Network (LAN) communication (HTTP / MQTT / WebSockets)

## 🚀 Quick Build & Flash
1. Navigate to the target project directory:
   ```bash
   cd firmware/esp32/esp32-rgb-node