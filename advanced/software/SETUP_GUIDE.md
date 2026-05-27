# 软件环境搭建指南（进阶方向）| Software Setup Guide (Advanced Track)

> 本文档详细说明进阶方向（Arduino + Python）的完整开发环境搭建步骤。
> This document details the complete development environment setup for the advanced track (Arduino + Python).

---

## 一、系统要求 | System Requirements

| 项目 | 最低要求 | 推荐配置 |
|------|---------|---------|
| 操作系统 (OS) | Windows 10 / macOS 12 / Ubuntu 20.04 | Windows 11 / macOS 14 |
| 磁盘空间 (Disk) | 5 GB | 10 GB+ |
| 内存 (RAM) | 4 GB | 8 GB+ |
| Python | 3.10 | 3.11 或 3.12 |
| Arduino IDE | 2.0+ | 2.3+ (最新稳定版) |
| USB 接口 | 至少 1 个 USB 2.0 | USB 3.0 |

---

## 二、Arduino IDE 安装 | Arduino IDE Installation

### 2.1 下载与安装

1. 访问 Arduino 官网下载页面：https://www.arduino.cc/en/software
2. 下载对应操作系统的安装包
   - **Windows**: 下载 `.exe` 安装器（推荐）或 `.zip` 便携版
   - **macOS**: 下载 `.dmg` 文件
   - **Linux**: 下载 `.AppImage` 或通过包管理器安装
3. 运行安装程序，按默认选项完成安装
4. 首次启动 Arduino IDE 2.x，等待索引更新完成

### 2.2 验证安装

```bash
# 打开 Arduino IDE，应该看到：
# - 菜单栏：File / Edit / Sketch / Tools / Help
# - 代码编辑区（空白 sketch）
# - 底部控制台输出区
# - 验证按钮（勾号图标）和上传按钮（箭头图标）
```

---

## 三、ESP32 开发板包安装 | ESP32 Board Package Installation

### 3.1 添加 ESP32 开发板管理器 URL

1. 打开 Arduino IDE
2. 进入 **File > Preferences**（macOS: **Arduino IDE > Settings**）
3. 在 **Additional boards manager URLs** 字段中添加以下 URL：

```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

如果你已经有其他 URL（如 ESP8266），用逗号或换行分隔多个 URL。

4. 点击 **OK** 保存设置

### 3.2 安装 ESP32 开发板包

1. 打开 **Tools > Board > Boards Manager...**（或左侧边栏的 "boards manager" 图标）
2. 搜索 **esp32**
3. 找到 **esp32 by Espressif Systems**，选择最新版本
4. 点击 **Install**，等待下载和安装完成（可能需要 5-10 分钟，取决于网速）
5. 安装完成后，关闭 Boards Manager

### 3.3 选择开发板

1. 进入 **Tools > Board > esp32**
2. 选择 **ESP32S3 Dev Module**（ESP32-S3 开发板）
3. 后续需要配置的参数见 [第九节：PSRAM 配置](#九psram-启用--psram-enable)

### 3.4 验证开发板包安装

```cpp
// 打开 File > Examples > 01.Basics > Blink
// 编译（点击验证按钮），应能成功编译
// 如果编译成功，说明 ESP32 开发板包安装正确

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}
```

---

## 四、Arduino 库安装 | Required Arduino Libraries

进阶方向需要安装以下库。在 Arduino IDE 中，通过 **Sketch > Include Library > Manage Libraries...** 搜索并安装。

The advanced track requires the following libraries. Install them via **Sketch > Include Library > Manage Libraries...** in Arduino IDE.

### 必需库列表 | Required Libraries

| 库名称 | 用途 | 安装方式 |
|--------|------|----------|
| **ESP32Servo** | 舵机控制（驱动桌面宠物四肢运动） | Library Manager 搜索 "ESP32Servo" |
| **TFT_eSPI** | SPI LCD 显示屏驱动（ST7789 等） | Library Manager 搜索 "TFT_eSPI" |
| **ESP32Camera** | OV2640 摄像头驱动 | Library Manager 搜索 "ESP32Camera" 或手动安装 |
| **ArduinoJson** | JSON 解析与生成（API 通信） | Library Manager 搜索 "ArduinoJson" |
| **WiFi** | Wi-Fi 连接（ESP32 内置，无需安装） | -- |
| **WebSockets** | WebSocket 通信（与后端实时交互） | Library Manager 搜索 "WebSockets by Markus Sattler" |
| **HTTPClient** | HTTP 请求（调用后端 API） | ESP32 内置，无需安装 |
| **ESP_I2S** | I2S 音频接口（麦克风录音、音频播放） | Library Manager 搜索 "ESP_I2S" |
| **Wire** | I2C 通信（传感器、音频编解码器） | ESP32 内置，无需安装 |
| **SPI** | SPI 通信（屏幕） | ESP32 内置，无需安装 |

### 库安装步骤

以 TFT_eSPI 为例：

1. 打开 **Sketch > Include Library > Manage Libraries...**
2. 在搜索框中输入 `TFT_eSPI`
3. 找到 **TFT_eSPI by Bodmer**，点击 **Install**
4. 等待安装完成
5. 重复以上步骤安装其他库

### 手动安装（备选方案）

如果 Library Manager 搜索不到某个库：

1. 从 GitHub 下载库的 ZIP 文件
2. 打开 **Sketch > Include Library > Add .ZIP Library...**
3. 选择下载的 ZIP 文件
4. 点击 Open 完成安装

### TFT_eSPI 配置

TFT_eSPI 库需要配置才能驱动你的屏幕。安装后需要编辑配置文件：

1. 找到 Arduino 库目录（通常在 `Documents/Arduino/libraries/TFT_eSPI/`）
2. 编辑 `User_Setup.h` 文件：
   - 注释掉不需要的驱动，取消注释 ST7789 驱动
   - 配置 SPI 引脚（根据你的接线图）
   - 设置屏幕分辨率（240x240）

```cpp
// User_Setup.h 关键配置示例
#define ST7789_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 240
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   10
#define TFT_DC    8
#define TFT_RST   9
#define TFT_BL    2
```

---

## 五、Python 安装 | Python 3.10-3.12 Installation

### 5.1 Windows

1. 访问 [Python 官网](https://www.python.org/downloads/) 下载 Python 3.11 或 3.12
2. 运行安装程序
3. **重要**：勾选 **"Add Python to PATH"**（将 Python 添加到环境变量）
4. 选择 **"Install Now"** 或自定义安装路径
5. 等待安装完成

```powershell
# 验证安装
python --version
# 应输出：Python 3.11.x 或 Python 3.12.x

pip --version
# 应输出 pip 版本信息
```

### 5.2 macOS

```bash
# 使用 Homebrew 安装
brew install python@3.11

# 验证安装
python3 --version
pip3 --version
```

### 5.3 Linux (Ubuntu/Debian)

```bash
# 安装 Python 3.11
sudo apt update
sudo apt install python3.11 python3.11-venv python3.11-dev python3-pip

# 验证安装
python3.11 --version
```

---

## 六、Python 虚拟环境设置 | Python venv Setup

虚拟环境用于隔离项目依赖，避免与系统 Python 包冲突。

Virtual environments isolate project dependencies from system Python packages.

```bash
# 1. 进入项目后端目录
cd project-backend/

# 2. 创建虚拟环境
# Windows:
python -m venv venv

# macOS/Linux:
python3 -m venv venv

# 3. 激活虚拟环境
# Windows (PowerShell):
venv\Scripts\Activate.ps1
# Windows (CMD):
venv\Scripts\activate.bat

# macOS/Linux:
source venv/bin/activate

# 4. 验证虚拟环境激活成功
# 命令行前缀应出现 (venv)
# 例如：(venv) C:\project-backend>

which python   # macOS/Linux: 应指向 venv/bin/python
where python   # Windows: 应指向 venv\Scripts\python.exe
```

**注意**：每次打开新的终端窗口都需要重新激活虚拟环境。

**Note**: You need to re-activate the virtual environment each time you open a new terminal.

---

## 七、后端依赖安装 | Backend Requirements Installation

### 7.1 安装依赖

```bash
# 确保虚拟环境已激活（命令行前缀显示 (venv)）
# Ensure venv is activated (prompt shows (venv))

# 安装所有依赖
pip install -r requirements.txt
```

### 7.2 requirements.txt 内容参考

```
# Web 框架
fastapi>=0.104.0
uvicorn>=0.24.0

# HTTP 客户端
httpx>=0.25.0

# 阿里云 DashScope SDK（通义千问多模态 API）
dashscope>=1.14.0

# 环境变量管理
python-dotenv>=1.0.0

# 音频处理
pydub>=0.25.1

# WebSocket
websockets>=12.0

# 异步支持
aiofiles>=23.0
```

### 7.3 验证安装

```bash
# 验证核心库安装成功
python -c "import fastapi; print('FastAPI OK')"
python -c "import dashscope; print('DashScope OK')"
python -c "import uvicorn; print('Uvicorn OK')"
```

---

## 八、.env 文件配置 | .env File Configuration

### 8.1 创建 .env 文件

在项目后端根目录下创建 `.env` 文件：

```bash
# 在项目根目录下创建 .env 文件
# Create .env file in the project root directory

# Windows:
copy .env.example .env

# macOS/Linux:
cp .env.example .env
```

### 8.2 配置 API Key

编辑 `.env` 文件，填入你的 API Key：

```env
# 阿里云 DashScope API Key（通义千问多模态 API）
# 获取地址：https://dashscope.console.aliyun.com/apiKey
DASHSCOPE_API_KEY=sk-your-api-key-here

# 服务器配置
HOST=0.0.0.0
PORT=8000

# 日志级别
LOG_LEVEL=INFO

# TTS 配置
TTS_VOICE=zhixiaoxia     # 可选语音角色
TTS_SAMPLE_RATE=16000     # 采样率
```

### 8.3 获取 DashScope API Key

1. 访问 [阿里云 DashScope 控制台](https://dashscope.console.aliyun.com/)
2. 注册/登录阿里云账号
3. 进入 **API-KEY 管理** 页面
4. 点击 **创建新的 API-KEY**
5. 复制生成的 API Key，粘贴到 `.env` 文件中

**重要**：`.env` 文件包含敏感信息，**不要提交到 Git 仓库**。确保 `.gitignore` 中包含 `.env`。

**Important**: The `.env` file contains sensitive information. **Never commit it to Git**. Ensure `.gitignore` includes `.env`.

---

## 九、分区方案配置 | Partition Scheme Setup

ESP32-S3 默认的分区方案可能不足以满足进阶方向的需求（摄像头、音频缓冲区需要较大 Flash 空间）。需要配置自定义分区表。

The default ESP32-S3 partition scheme may not have enough space for advanced track needs (camera, audio buffers require larger Flash). Custom partition tables are needed.

### 9.1 创建自定义分区表文件

在 Arduino sketch 目录下创建 `partitions.csv` 文件：

```csv
# Name,    Type, SubType, Offset,   Size,    Flags
nvs,       data, nvs,     ,         0x6000,
phy_init,  data, phy,     ,         0x1000,
factory,   app,  factory, ,         0x40000,
storage,   data, spiffs,  ,         0x10000,
```

### 9.2 配置 Arduino IDE 使用自定义分区表

1. 在 **Tools** 菜单中找到 **Partition Scheme**
2. 如果有 **"Custom"** 选项，选择它
3. 如果没有 Custom 选项：
   - 将 `partitions.csv` 文件放在 sketch 目录下（与 `.ino` 文件同级）
   - 或将分区文件放到 ESP32 开发板包的 `tools/partitions/` 目录中

### 9.3 替代方案：使用预设分区方案

在 **Tools > Partition Scheme** 中选择：

- **"Huge APP (3MB APP/1.5MB SPIFFS)"** -- 如果你的 Flash 是 16MB
- **"Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"** -- 如果你的 Flash 是 4MB

---

## 十、PSRAM 启用 | PSRAM Enable

ESP32-S3-WROOM-1-N16R8 模组内置 8MB PSRAM，对于摄像头和音频处理至关重要。

The ESP32-S3-WROOM-1-N16R8 module has 8MB PSRAM, which is critical for camera and audio processing.

### 10.1 在 Arduino IDE 中启用 PSRAM

1. 进入 **Tools > Board > esp32 > ESP32S3 Dev Module**
2. 设置以下参数：

| 配置项 | 设置值 | 说明 |
|--------|--------|------|
| **Flash Size** | 16MB (128Mb) | 匹配 N16 型号 |
| **Partition Scheme** | Huge APP (3MB) 或 Custom | 确保有足够 APP 空间 |
| **PSRAM** | OPI PSRAM | 启用 8MB PSRAM |
| **Upload Speed** | 921600 | 快速上传速度 |
| **USB CDC On Boot** | Enabled | 通过 USB 查看串口输出 |
| **Port** | 选择对应 COM 口 | Windows: COMx / macOS: /dev/cu.usbmodem* |

### 10.2 验证 PSRAM

```cpp
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.printf("Total heap: %d\n", ESP.getHeapSize());
    Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
    Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());

    // 应该能看到 Total PSRAM: 8388608 (8MB)
}

void loop() {}
```

---

## 十一、首次固件上传 | First Firmware Upload

### 11.1 连接硬件

1. 使用 USB Type-C 数据线将 ESP32-S3 开发板连接到电脑
2. 确认数据线支持数据传输（非仅充电线）
3. 等待电脑识别设备

### 11.2 检查端口

```powershell
# Windows: 打开设备管理器，查看 "端口 (COM 和 LPT)"
# 应看到类似 "USB Serial Device (COM3)" 或 "Silicon Labs CP210x (COM5)"

# macOS/Linux:
ls /dev/cu.usb*
# 应看到类似 /dev/cu.usbmodem1401
```

### 11.3 上传 Blink 测试

1. 打开 **File > Examples > 01.Basics > Blink**
2. 选择 **Tools > Board > ESP32S3 Dev Module**
3. 选择 **Tools > Port** 中对应的 COM 口
4. 点击上传按钮（箭头图标）
5. 等待编译和上传完成
6. 观察 ESP32 上的 LED 灯是否开始闪烁

### 11.4 串口监视器

1. 打开 **Tools > Serial Monitor**
2. 设置波特率为 **115200**
3. 应能看到 ESP32 的启动日志输出

### 11.5 常见上传问题

| 问题 | 解决方法 |
|------|----------|
| "Failed to connect to ESP32" | 按住 BOOT 键，点击 EN 键，松开 BOOT 键 |
| 端口不显示 | 检查 USB 线是否支持数据传输；安装 CH340/CP210x 驱动 |
| 上传成功但无反应 | 按一下 EN（Reset）键重启 |
| 编译报错找不到库 | 确认库已通过 Library Manager 安装 |

---

## 十二、后端服务器启动 | Backend Server Startup

### 12.1 启动服务器

```bash
# 1. 确保在项目后端目录
cd project-backend/

# 2. 激活虚拟环境
# Windows:
venv\Scripts\activate
# macOS/Linux:
source venv/bin/activate

# 3. 启动服务器
python -m uvicorn app:app --host 0.0.0.0 --port 8000 --reload

# 或使用 main.py 启动
python main.py
```

### 12.2 验证服务器

```bash
# 在浏览器中打开以下地址
# Open these URLs in your browser

# 健康检查
http://localhost:8000/health
# 应返回：{"status": "ok"}

# API 文档（FastAPI 自动生成）
http://localhost:8000/docs
# 应显示 Swagger UI 界面

# 替代 API 文档
http://localhost:8000/redoc
```

### 12.3 常见启动问题

| 问题 | 解决方法 |
|------|----------|
| `ModuleNotFoundError` | 运行 `pip install -r requirements.txt` |
| `DASHSCOPE_API_KEY not found` | 检查 `.env` 文件是否存在且配置正确 |
| `Address already in use` | 端口 8000 被占用，改用 `--port 8001` |
| `Permission denied` | macOS/Linux 上使用 `sudo` 或选择 > 1024 的端口 |

---

## 十三、完整环境验证清单 | Full Environment Verification Checklist

完成以上所有步骤后，请逐项验证：

After completing all steps above, verify each item:

### Arduino 环境

- [ ] Arduino IDE 安装完成并可正常启动
- [ ] ESP32 开发板包安装成功（Boards Manager 中可见）
- [ ] 能选择 **ESP32S3 Dev Module** 开发板
- [ ] Blink 示例编译成功
- [ ] Blink 示例上传到 ESP32 成功
- [ ] LED 灯闪烁正常
- [ ] 串口监视器能输出日志
- [ ] PSRAM 检测到 8MB
- [ ] 所有必需库安装完成

### Python 环境

- [ ] Python 3.10-3.12 安装成功
- [ ] venv 虚拟环境创建成功
- [ ] 虚拟环境能正常激活
- [ ] requirements.txt 依赖安装成功
- [ ] `.env` 文件已创建并填入 API Key
- [ ] 后端服务器能正常启动
- [ ] `/health` 端点返回正常
- [ ] `/docs` 页面能正常显示

### 硬件连接

- [ ] ESP32-S3 通过 USB 连接到电脑
- [ ] COM 端口正确识别
- [ ] 固件上传正常
- [ ] 屏幕显示正常（如已焊接）
- [ ] 舵机运动正常（如已焊接）

---

*环境搭建完成后，即可进入项目开发。*
*Once the environment is set up, you can start project development.*

*最后更新：2026-05-27*
