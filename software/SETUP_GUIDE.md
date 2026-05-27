# ESP-SparkBot 开发环境搭建指南
# ESP-SparkBot Development Environment Setup Guide

> 本文档替代原来的 `requirements.txt`，详细说明 ESP-IDF 嵌入式开发环境搭建。
> This document replaces the original `requirements.txt`, detailing the ESP-IDF embedded development environment setup.

---

## 一、系统要求 | System Requirements

| 项目 | 最低要求 | 推荐配置 |
|------|---------|---------|
| 操作系统 (OS) | Windows 10 / Ubuntu 20.04 / macOS 12 | Windows 11 / Ubuntu 24.04 |
| 磁盘空间 (Disk) | 10 GB | 20 GB+ |
| 内存 (RAM) | 8 GB | 16 GB |
| Python | 3.8 | 3.11+ |
| USB 接口 | 至少 1 个 USB 2.0 | 2 个 USB 3.0 |

---

## 二、ESP-IDF 安装 | ESP-IDF Installation

### 2.1 Windows (推荐使用安装器 | Installer Recommended)

```powershell
# 方法一：使用 ESP-IDF 在线安装器（最简单）
# Method 1: Use ESP-IDF Online Installer (easiest)
# 下载地址 | Download: https://dl.espressif.com/dl/esp-idf/

# 安装器会自动完成：
# The installer will automatically:
# 1. 安装 ESP-IDF v5.3.2 源代码
# 2. 安装所有编译工具链（xtensa-esp32s3-elf-gcc 等）
# 3. 安装 Python 3.11 + pip 依赖
# 4. 安装 CMake + Ninja 构建工具
# 5. 安装 VS Code 的 ESP-IDF 插件（可选）
# 6. 安装 USB 驱动（CH340, CP210x, FTDI）
```

```powershell
# 方法二：使用 PowerShell 脚本安装（离线安装）
# Method 2: Offline installer via PowerShell script

# 1. 克隆 ESP-IDF 仓库
git clone --recursive https://github.com/espressif/esp-idf.git -b v5.3.2 C:\esp-idf

# 2. 运行安装脚本
C:\esp-idf\install.bat esp32s3

# 3. 每次打开终端前设置环境变量
C:\esp-idf\export.bat
```

### 2.2 Linux / macOS

```bash
# 安装系统依赖 (Install system dependencies)
# Ubuntu/Debian:
sudo apt-get install git wget flex bison gperf python3 python3-pip \
  python3-venv cmake ninja-build ccache libffi-dev libssl-dev \
  dfu-util libusb-1.0-0

# macOS:
brew install cmake ninja dfu-util

# 克隆 ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git -b v5.3.2 ~/esp-idf

# 安装工具链
cd ~/esp-idf
./install.sh esp32s3

# 设置环境变量（建议加入 .bashrc / .zshrc）
source ~/esp-idf/export.sh
```

---

## 三、Python 依赖 | Python Dependencies

```bash
# ESP-IDF 自带的 Python 虚拟环境会自动安装以下包：
# ESP-IDF's built-in Python venv auto-installs:

pip install -r $IDF_PATH/requirements.txt

# 额外建议的 Python 包（用于串口调试等）
# Additional recommended packages:
pip install pyserial esptool

# 验证安装 | Verify installation
python -c "import esptool; print('esptool OK')"
```

---

## 四、VS Code 扩展 | VS Code Extensions

在 VS Code 中搜索并安装以下扩展：

| 扩展名 | 用途 |
|--------|------|
| **ESP-IDF** (by Espressif) | 构建、烧录、监控一站式支持 |
| **C/C++** (by Microsoft) | C 语言代码补全和导航 |
| **Cortex-Debug** | ARM 调试支持 |
| **Serial Monitor** | 串口监视器 |
| **CMake Tools** | CMake 构建支持 |
| **Chinese (Simplified)** | 中文界面包（可选） |

---

## 五、USB 驱动安装 | USB Driver Installation

### 5.1 CH340 驱动

ESP-SparkBot 主控板的 USB 转串口芯片通常是 CH340C。

- **Windows**: 下载 CH340 驱动 [WCH官网](http://www.wch.cn/download/CH341SER_EXE.html)
  ```powershell
  # 安装后在设备管理器中检查
  # Check in Device Manager after installation
  devmgmt.msc
  # 查找：端口 (COM 和 LPT) → USB-SERIAL CH340 (COMx)
  ```
- **Linux**: 内核自带驱动，无需额外安装 (kernel built-in, no extra install needed)
- **macOS**: `brew install wch-ch34x-usb-serial-driver`

### 5.2 CP210x 驱动

如果使用的是 CP210x 芯片的开发板。

- **下载**: [SiLabs CP210x 驱动](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)

---

## 六、编译与烧录 | Build & Flash

```bash
# 进入项目目录
cd esp_sparkbot

# 设置目标芯片（ESP32-S3）
idf.py set-target esp32s3

# 配置项目（打开 menuconfig 图形界面）
idf.py menuconfig
# 需要配置：
# 1. WiFi SSID 和密码 (WiFi SSID & Password)
# 2. AI API token (DeepSeek / 文心一言 / 火山引擎)
# 3. 百度语音 token (Baidu STT/TTS)
# 4. 显示屏 SPI 模式 (Display SPI mode)
# 5. 摄像头参数 (Camera vflip, hmirror)

# 编译项目
idf.py build

# 烧录到 ESP32-S3（将设备连接到 USB，检查端口号）
# Flash to ESP32-S3 (connect device via USB, check COM port)
idf.py -p COM3 flash
# Linux/macOS:  idf.py -p /dev/ttyUSB0 flash

# 查看串口日志
idf.py -p COM3 monitor

# 退出串口监控: Ctrl + ]
# 一键编译+烧录+监控: idf.py -p COM3 flash monitor
```

---

## 七、项目配置参数参考 | Project Configuration Reference

| 配置项 (Kconfig) | 说明 | 示例值 |
|---|---|---|
| `SPARKBOT_WIFI_SSID` | WiFi 名称 | "MyWiFi" |
| `SPARKBOT_WIFI_PASSWORD` | WiFi 密码 | "password123" |
| `SPARKBOT_AI_PROVIDER` | AI 提供商 | deepseek / wenxin / huoshan / openai |
| `SPARKBOT_AI_API_KEY` | AI API 密钥 | "sk-xxxxx" |
| `SPARKBOT_TTS_TOKEN` | 百度语音合成 token | "24.xxxxx.xxxxx" |
| `SPARKBOT_DISPLAY_SPI_MODE` | 显示屏 SPI 模式 | 0-3 |
| `SPARKBOT_CAMERA_VFLIP` | 摄像头垂直翻转 | y/n |
| `SPARKBOT_CAMERA_HMIRROR` | 摄像头水平镜像 | y/n |

---

## 八、常见问题 | Troubleshooting

### Q1: `idf.py` 命令找不到？

```powershell
# Windows: 确保运行了 export.bat
C:\esp-idf\export.bat

# Linux/macOS:
source ~/esp-idf/export.sh
```

### Q2: 烧录失败 "A fatal error occurred: Failed to connect to ESP32-S3"？

```bash
# 1. 检查 USB 线是否支持数据传输（有些线只充电）
# 2. 按住 BOOT 键不放，按一下 EN 键，再松开 BOOT 键（进入下载模式）
# 3. 检查串口号是否正确
# Windows: 使用设备管理器查看COM端口号
# Linux: ls /dev/ttyUSB* 或 ls /dev/ttyACM*
```

### Q3: 编译报错 "CMAKE_MAKE_PROGRAM is not set"？

```bash
# 需要安装 ninja-build
# Ubuntu: sudo apt install ninja-build
# Windows: ESP-IDF 安装器已自带
# macOS: brew install ninja
```

### Q4: 运行时不断重启 (Watchdog reset)？

- 检查供电是否足够（ESP32-S3 峰值电流可达 500mA）
- 检查任务栈大小是否足够（增加 `configMINIMAL_STACK_SIZE`）
- 检查是否在中断处理函数中调用了阻塞 API

---

*环境搭建完成后，即可进入项目开发。*
*Once the environment is set up, you can start project development.*
