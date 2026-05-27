# AICat 进阶版 | AICat Advanced — AI桌面猫（多模态大模型）| Multimodal AI Desktop Cat

> 基于云端多模态大模型 Qwen 3.5 Omni 的可步行桌面机器猫 | A Walking Desktop Robot Cat Powered by Cloud-based Multimodal LLM Qwen 3.5 Omni

**原始项目**: [AI-FanGe/AI_DesktopCat_Qwen3.5Omni](https://github.com/AI-FanGe/AI_DesktopCat_Qwen3.5Omni) | **开源协议**: MIT

---

## 与基础版的区别 | Differences from Base Track (Track A)

| 对比项 | 基础版 (Track A) — ESP-SparkBot | 进阶版 (Track B) — AICat |
|--------|--------------------------------|--------------------------|
| 主控板 | ESP32-S3-WROOM-1-N16R8（自焊PCB） | Seeed XIAO ESP32S3 Sense（成品开发板） |
| 开发框架 | ESP-IDF (C) | Arduino IDE (C/C++) |
| AI 方案 | 端侧语音识别 + 云端文本 LLM | 云端多模态大模型 Qwen 3.5 Omni（语音/视觉/文本） |
| 语音输入 | ESP-SR 本地语音命令 | 板载麦克风 + 后端 ASR + 大模型对话 |
| 摄像头 | OV2640（人脸识别/移动检测） | 板载摄像头（实时推流到网页） |
| 显示 | 1.54寸 240x240 ST7789 (LVGL) | 1.83寸 170x320 ST7789 (LittleFS 表情帧) |
| 运动能力 | 静态桌面摆件（可选履带底盘） | 4条腿可步行（STS3032 总线舵机） |
| 表情系统 | LVGL 图形界面 | LittleFS 存储的表情动画帧 |
| 音频输出 | ES8311 编解码器 + NS4150B 功放 | MAX98357A I2S 功放模块 |
| 舵机控制 | 无（或履带底盘电机） | PCA9685 PWM（嘴/尾/耳）+ STS3032 总线（四腿） |
| 后端服务 | 无（ESP32 直连 LLM API） | Python 后端 + WebSocket 中转 + Web 控制面板 |
| 焊接要求 | 从零焊接整块 PCB | 杜邦线/面包板接线，无需焊接 |
| 编程语言 | C | C/C++（固件）+ Python（后端） |

**核心区别总结 | Summary**: 基础版侧重嵌入式系统开发（从 PCB 焊接到驱动编写），进阶版侧重系统集成与云端 AI 交互（Arduino 快速原型 + Python 后端 + 多模态大模型）。进阶版有可步行四足结构和丰富的表情动画系统。

---

## 项目简介 | Project Overview

AICat 进阶版是一个基于 Seeed XIAO ESP32S3 Sense 的桌面机器猫项目。与基础版不同，进阶版通过 WebSocket 连接到运行在电脑上的 Python 后端服务器，后端调用阿里云 DashScope 的 Qwen 3.5 Omni 多模态大模型，实现语音对话、图像理解和智能决策。

ESP32 端负责硬件控制：ST7789 屏幕显示表情动画（存储在 LittleFS 中）、板载摄像头实时推流、板载麦克风采集语音、MAX98357A 播放 AI 回复、PCA9685 驱动嘴/尾巴/耳朵舵机、STS3032 总线舵机驱动四条腿完成步行/坐下/跳跃等动作。Python 后端提供 Web 控制面板，可以在浏览器里查看摄像头画面、测试语音输入、发送表情命令和调试舵机。

AICat Advanced is a desktop robot cat project based on the Seeed XIAO ESP32S3 Sense. Unlike the base track, the advanced track connects to a Python backend server running on your computer via WebSocket. The backend calls Alibaba Cloud's DashScope Qwen 3.5 Omni multimodal LLM for voice conversation, image understanding, and intelligent decision-making.

The ESP32 handles hardware control: ST7789 screen for facial expression animations (stored in LittleFS), onboard camera for real-time streaming, onboard microphone for voice capture, MAX98357A for playing AI responses, PCA9685 for mouth/tail/ear servos, and STS3032 bus servos for four-legged walking/sitting/jumping. The Python backend provides a web control panel where you can view the camera feed, test voice input, send expression commands, and debug servos -- all from a browser.

| 项目 | 开源地址 |
|------|----------|
| 软件源码（含固件 + Python 后端 + 3D 打印文件） | https://github.com/AI-FanGe/AI_DesktopCat_Qwen3.5Omni |

## 最终效果 | Final Result

完成本项目后，你将拥有：
- 一只能走路的桌面机器猫（四足步行、坐下、跳跃等动作）
- 屏幕表情动画系统（眼睛、嘴部、多种情绪表情通过 LittleFS 存储）
- 实时摄像头推流到浏览器（XIAO ESP32S3 Sense 板载摄像头）
- 语音输入与 AI 对话（板载麦克风 + Qwen 3.5 Omni 多模态大模型）
- 扬声器播放 AI 语音回复（MAX98357A I2S 功放）
- 嘴巴/尾巴/耳朵舵机动作（PCA9685 PWM 驱动）
- Web 控制面板（浏览器中调试所有功能）

After completing this project, you will have:
- A walking desktop robot cat (four-legged walking, sitting, jumping, and more)
- Screen facial expression animation system (eyes, mouth, multiple emotion expressions stored in LittleFS)
- Real-time camera streaming to browser (XIAO ESP32S3 Sense onboard camera)
- Voice input and AI conversation (onboard mic + Qwen 3.5 Omni multimodal LLM)
- Speaker playback of AI voice responses (MAX98357A I2S amplifier)
- Mouth / tail / ear servo actions (PCA9685 PWM driver)
- Web control panel (debug all features from a browser)

## 项目能做什么 | What It Can Do

- 用 ST7789 小屏显示状态、文字和表情动画（JPEG 帧序列存于 LittleFS）
- 用 XIAO ESP32S3 Sense 板载摄像头把画面实时传到电脑端网页
- 用板载麦克风采集语音，交给后端做 ASR 和 AI 对话（Qwen 3.5 Omni）
- 用 MAX98357A 播放 AI 回复或提示音
- 用 PCA9685 控制嘴巴、尾巴、耳朵等普通 PWM 舵机（CH12-CH15）
- 用 STS3032 总线舵机控制四条腿，支持走路、坐下、跳跃等动作（ID1-ID4）
- 用浏览器打开后端控制台，调试视频、语音、表情和舵机

- Display status, text, and facial expression animations on the ST7789 screen (JPEG frame sequences stored in LittleFS)
- Stream real-time camera footage to a web page via the XIAO ESP32S3 Sense onboard camera
- Capture voice via the onboard microphone and send it to the backend for ASR and AI conversation (Qwen 3.5 Omni)
- Play AI responses or notification sounds through the MAX98357A
- Control mouth, tail, and ear PWM servos via the PCA9685 (CH12-CH15)
- Control four legs via STS3032 bus servos for walking, sitting, jumping, and more (ID1-ID4)
- Open the backend control console in a browser to debug video, voice, expressions, and servos

## 核心技术 | Core Technologies

| 技术 | 用途 | 说明 |
|------|------|------|
| Seeed XIAO ESP32S3 Sense | 主控板 | ESP32-S3 双核 240MHz, 8MB PSRAM, 板载 OV2640 摄像头 + 麦克风 |
| Arduino IDE | 开发框架 | C/C++ 开发，比 ESP-IDF 更易上手 |
| ST7789 (SPI LCD) | 显示驱动 | 1.83寸 170x320 圆角 IPS 屏，用于表情动画显示 |
| LittleFS | 文件系统 | ESP32 片上 Flash 存储，存放表情/嘴部动画的 JPEG 帧 |
| PCA9685 | PWM 舵机驱动 | 16 路 I2C PWM 驱动，控制嘴巴/尾巴/耳朵（CH12-CH15） |
| STS3032 | 总线舵机 | TTL 串行总线舵机，驱动四条腿（ID1-ID4），1000000 波特率 |
| MAX98357A | I2S 音频功放 | 数字输入 D 类功放，驱动小喇叭播放语音 |
| Qwen 3.5 Omni | 多模态大模型 | 阿里云 DashScope API，支持语音/视觉/文本多模态输入输出 |
| WebSocket | 实时通信 | ESP32 与 Python 后端的双向通信（摄像头流、音频流、命令） |
| Python 后端 | 服务中转 | 接收 ESP32 数据、调用 AI API、提供 Web 控制面板 |

## 硬件清单 | Hardware List

### 主控与核心元件 | Main Controller & Core Components

| 部件 | 规格 | 参考价格 | 说明 |
|------|------|----------|------|
| 主控板 | Seeed XIAO ESP32S3 Sense | ~55元 | 含板载摄像头 + 麦克风，必须买 Sense 版本 |
| LCD 屏幕 | 1.83寸 SPI, 170x320, ST7789 | ~20元 | 圆角 IPS 屏，SPI 接口 |
| 音频功放 | MAX98357A I2S 模块 | ~8元 | 数字输入 D 类功放 |
| 小喇叭 | 4Ω/3W | ~5元 | 配合 MAX98357A 使用 |
| PWM 舵机驱动 | PCA9685 16路 | ~10元 | I2C 接口，驱动嘴巴/尾巴/耳朵舵机 |

### 舵机 | Servos

| 部件 | 规格 | 参考价格 | 说明 |
|------|------|----------|------|
| STS3032 总线舵机 | x4 | ~240元（60元/个） | TTL 总线舵机，控制四条腿（ID1-ID4） |
| TTL/总线转接模块 | x1 | ~15元 | STS3032 通信转接板 |
| MG90S 金属齿轮舵机 | x1 | ~8元 | 尾巴舵机，力矩较大 |
| 2g/3.7g 塑料齿轮舵机 | x3 | ~12元（4元/个） | 嘴巴 + 左耳 + 右耳，180度 PWM |

### 电源与配件 | Power & Accessories

| 部件 | 规格 | 参考价格 | 说明 |
|------|------|----------|------|
| 舵机电源 | 5V/2A 以上独立电源 | ~15元 | 舵机供电必须独立，不能从 ESP32 取电 |
| 杜邦线 | 公对公 / 公对母 若干 | ~5元 | 各模块接线 |
| 面包板 | 400孔（可选） | ~5元 | 原型搭建用 |
| USB 数据线 | Type-C | ~5元 | XIAO 开发板使用 Type-C 接口 |

### 3D 打印外壳 | 3D Printed Enclosure

| 部件 | 说明 |
|------|------|
| 猫身体外壳 | 项目仓库包含 STL 文件，PLA/PETG 打印 |
| 腿部结构件 | 配合 STS3032 舵机 |
| 头部结构件 | 固定屏幕、摄像头 |

**总成本约 400-450 元**（含 STS3032 舵机，不含 3D 打印机和工具），在 500 元预算以内。

**Total cost approx. 400-450 CNY** (including STS3032 servos, excluding 3D printer and tools), within the 500 CNY budget.

## 软件环境 | Software Environment

| 软件 | 版本 | 用途 |
|------|------|------|
| Arduino IDE | 2.x | ESP32 固件开发、编译、烧录 |
| ESP32 Arduino Core | latest | Arduino IDE 中的 ESP32 开发板支持包 |
| Python | 3.10 - 3.12 | 后端服务器运行环境 |
| pip | latest | Python 包管理 |
| Git | latest | 克隆项目仓库 |
| 浏览器 | Chrome / Edge | 打开 Web 控制面板 |

### Arduino 库依赖 | Arduino Library Dependencies

| 库名 | 用途 |
|------|------|
| ESP32Servo | ESP32 舵机控制 |
| Adafruit GFX Library | 图形绘制基础库 |
| Adafruit ST7735 and ST7789 Library | ST7789 屏幕驱动 |
| Adafruit PWM Servo Driver Library | PCA9685 PWM 驱动 |
| ArduinoWebsockets | WebSocket 客户端通信 |
| JPEGDEC | JPEG 解码（表情帧渲染） |
| SCServo | STS3032 总线舵机控制 |

## 快速开始 | Quick Start

### Step 1: 克隆项目 | Clone the Project

```bash
git clone https://github.com/AI-FanGe/AI_DesktopCat_Qwen3.5Omni.git
cd AI_DesktopCat_Qwen3.5Omni
```

### Step 2: 配置 Arduino IDE | Set Up Arduino IDE

1. 安装 [Arduino IDE](https://www.arduino.cc/en/software)（2.x 版本）
2. 打开 `Arduino IDE -> Settings`，在 `Additional boards manager URLs` 中添加：
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. 打开 `Tools -> Board -> Boards Manager`，搜索 `esp32`，安装 `esp32 by Espressif Systems`
4. 安装所需 Arduino 库（见上方库依赖表）
5. 选择开发板：`XIAO ESP32S3`
6. 启用 PSRAM：`Tools -> PSRAM -> Enabled`
7. 启用 USB CDC：`Tools -> USB CDC On Boot -> Enabled`

### Step 3: 配置 Python 后端 | Set Up Python Backend

```bash
cd upload_facial_expression/integrated/server

# 创建虚拟环境
python -m venv .venv

# 激活虚拟环境
# macOS / Linux:
source .venv/bin/activate
# Windows PowerShell:
.venv\Scripts\Activate.ps1

# 安装依赖
pip install -r requirements.txt

# 配置 API Key（阿里云 DashScope）
# 创建 .env 文件，写入：
# DASHSCOPE_API_KEY=sk-your-api-key-here
# ASR_DEBUG_RAW=0
```

启动后端：

```bash
python app.py
# 默认监听 0.0.0.0:8081
# 在浏览器中打开 http://你的电脑IP:8081
```

### Step 4: 配置 ESP32 固件 | Configure ESP32 Firmware

打开 `upload_facial_expression/integrated/integrated.ino`，修改以下配置：

```cpp
const char* WIFI_SSID   = "YOUR_WIFI_SSID";      // 你的 Wi-Fi 名称
const char* WIFI_PASS   = "YOUR_WIFI_PASSWORD";  // 你的 Wi-Fi 密码
const char* SERVER_HOST = "192.168.x.x";         // 运行后端的电脑局域网 IP
const uint16_t SERVER_PORT = 8081;                // 后端端口
```

Arduino IDE 推荐设置：
- Board: `XIAO ESP32S3`
- PSRAM: `Enabled`
- USB CDC On Boot: `Enabled`
- Partition Scheme: `Custom`（使用项目自带的 `partitions.csv`）

点击上传。

### Step 5: 上传表情素材 | Upload Facial Expression Assets

表情素材需要分步写入 ESP32 的 LittleFS 文件系统：

1. 准备表情帧素材（JPEG 格式，170x320 或更小），放入 `upload_facial_expression/data/` 目录
2. 运行生成脚本：
   ```bash
   cd upload_facial_expression/flash_files
   python generate_all_headers.py
   ```
3. 烧录写入工具 `flash_files.ino`，通过串口发送 `F`（格式化）和 `W`（写入）
4. 如有多个批次，修改 `BATCH_NUMBER` 后重复上传和写入
5. 嘴部表情素材使用 `mouth_flash_files/` 目录下的工具，流程相同
6. 完成后重新烧录主固件 `integrated.ino`

**注意**: 写入工具和主固件必须使用同一套分区表。更改分区表会清空 Flash 数据，需要重新上传表情。

### Step 6: 硬件接线 | Wiring

按照以下接线表连接各模块：

**ST7789 屏幕**:
- CS -> D0, DC -> D1, RST -> EN, SCK -> D8, MOSI -> D10, VCC -> 3.3V, GND -> GND

**PCA9685 舵机驱动**:
- SDA -> D4, SCL -> D5, VCC -> 3.3V, V+ -> 外部舵机电源, GND -> 公共地
- CH12=嘴巴, CH13=尾巴, CH14=左耳, CH15=右耳

**STS3032 总线舵机**:
- TX -> D6 (GPIO43), RX -> D7 (GPIO44), 波特率 1000000
- ID1=左前腿, ID2=右前腿, ID3=左后腿, ID4=右后腿

**MAX98357A 扬声器**:
- BCLK -> D3 (GPIO4), LRC -> D2 (GPIO3), DIN -> D9 (GPIO8), VCC -> 5V, GND -> 公共地

**重要**: 所有外部电源的 GND 必须与 ESP32 的 GND 共地。舵机电流较大，供电不稳会导致重启。

### Step 7: 第一次联调 | First Integration Test

建议按以下顺序逐步验证：

1. 启动 Python 后端，确认浏览器能打开 `http://电脑IP:8081`
2. 给 ESP32 上电，串口确认连上 Wi-Fi
3. 查看后端终端是否出现 camera/audio WebSocket 连接日志
4. 网页中查看摄像头画面
5. 测试屏幕文字或表情命令
6. 单独测试 PCA9685 上的嘴巴/尾巴/耳朵舵机
7. 单独测试 STS3032 四腿舵机，确认 ID 和方向正确
8. 最后测试语音输入、AI 回复和扬声器播放

## 项目结构 | Project Structure

```
AI_DesktopCat_Qwen3.5Omni/
├── README.md                                    # 项目说明文档
├── docs/                                        # 文档与图片
│   └── images/                                  # 教程图片（成品照、接线图、物料图）
├── upload_facial_expression/                    # 主固件与表情系统
│   ├── integrated/                              # 主固件
│   │   ├── integrated.ino                       # ESP32 主固件（Arduino）
│   │   ├── partitions.csv                       # 自定义分区表
│   │   └── server/                              # Python 后端
│   │       ├── app.py                           # 后端主程序
│   │       ├── requirements.txt                 # Python 依赖
│   │       └── .env                             # API Key 配置（不入库）
│   ├── flash_files/                             # 表情帧写入工具
│   │   ├── flash_files.ino                      # 写入固件
│   │   ├── generate_all_headers.py              # 表情帧头文件生成脚本
│   │   ├── video_to_jpeg_frames.py              # 视频转 JPEG 帧脚本
│   │   ├── audio_to_adpcm.py                    # 音频转 ADPCM 脚本
│   │   └── partitions.csv                       # 写入工具分区表
│   ├── mouth_flash_files/                       # 嘴部表情写入工具
│   │   ├── mouth_flash_files.ino                # 嘴部写入固件
│   │   ├── generate_mouth_headers.py            # 嘴部帧头文件生成脚本
│   │   └── video_to_jpeg_frames.py              # 视频转帧脚本
│   ├── data/                                    # 表情帧素材目录（需自备）
│   │   ├── anim1/                               # 动画1（0001.jpg, 0002.jpg, ...）
│   │   ├── anim2/                               # 动画2
│   │   └── ...                                  # 更多动画
│   ├── mouth_flash_files/data/                  # 嘴部素材目录
│   │   ├── mouth_closed/                        # 闭嘴
│   │   ├── mouth_small_open/                    # 微张
│   │   ├── mouth_big_open/                      # 大张
│   │   ├── mouth_wide/                          # 宽嘴
│   │   └── mouth_round/                         # 圆嘴
│   └── partitions_custom.csv                    # 自定义分区表参考
└── 3D models/                                   # 3D 打印外壳 STL 文件
```

## 课程安排 | Course Schedule

### 第一阶段：环境搭建与硬件组装（Day 1-4）
Phase 1: Environment Setup & Hardware Assembly

| 天数 | 主题 | 核心知识点 |
|------|------|-----------|
| Day 1 | XIAO ESP32S3 平台入门与 Arduino IDE 搭建 | Arduino IDE 安装、ESP32 开发板支持包、库安装、Blink 点灯测试、串口监视器 |
| Day 2 | 硬件认知与接线实践 | XIAO ESP32S3 Sense 引脚图、ST7789 屏幕/PCA9685/MAX98357A 接线、面包板原型搭建 |
| Day 3 | STS3032 总线舵机与 PCA9685 PWM 舵机 | I2C 通信（PCA9685）、TTL 总线协议（STS3032）、舵机角度校准、空载测试 |
| Day 4 | 3D 打印外壳组装与整机组装 | 外壳打印与处理、舵机安装固定、接线整理、首次上电全模块检测 |

### 第二阶段：固件开发与表情系统（Day 5-8）
Phase 2: Firmware Development & Expression System

| 天数 | 主题 | 核心知识点 |
|------|------|-----------|
| Day 5 | ST7789 屏幕驱动与基础显示 | SPI 通信协议、Adafruit GFX 绘图、文字/图形/颜色显示、屏幕坐标系统 |
| Day 6 | LittleFS 文件系统与表情动画 | LittleFS 挂载与读写、JPEG 帧序列播放、表情帧生成工具链、批次写入流程 |
| Day 7 | 摄像头推流与 WebSocket 通信 | OV2640 初始化与采集、JPEG 编码、WebSocket 客户端实现、实时帧传输 |
| Day 8 | I2S 音频播放与 PCA9685 动作控制 | MAX98357A I2S 驱动、音频数据流、PCA9685 PWM 控制、嘴/尾/耳动作编排 |

### 第三阶段：AI 集成与项目展示（Day 9-12）
Phase 3: AI Integration & Project Demo

| 天数 | 主题 | 核心知识点 |
|------|------|-----------|
| Day 9 | Python 后端搭建与 Web 控制面板 | Flask/FastAPI 服务、WebSocket 服务端、HTML 控制面板、摄像头/音频中转 |
| Day 10 | Qwen 3.5 Omni 多模态 AI 集成 | DashScope API 调用、语音识别（ASR）、多模态对话、TTS 语音合成、AI 回复播放 |
| Day 11 | STS3032 四足步态与动作编排 | 步行步态算法、坐下/跳跃动作、情绪-动作映射、完整系统联调 |
| Day 12 | 项目展示与总结 | 功能演示、技术复盘、Bug 修复经验、拓展方向讨论（电池供电、网页配网、情绪系统） |

## 学习资源 | Learning Resources

### 官方文档 | Official Documentation

| 资源 | 说明 |
|------|------|
| [AI_DesktopCat_Qwen3.5Omni 仓库](https://github.com/AI-FanGe/AI_DesktopCat_Qwen3.5Omni) | 项目源码、接线图、3D 模型 |
| [Seeed XIAO ESP32S3 Sense Wiki](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/) | 官方入门指南与引脚定义 |
| [Arduino ESP32 官方文档](https://docs.espressif.com/projects/arduino-esp32/) | Arduino 框架下的 ESP32 开发文档 |
| [DashScope API 文档](https://help.aliyun.com/zh/model-studio/) | 阿里云 DashScope / Qwen API 文档 |
| [PCA9685 数据手册](https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf) | PWM 舵机驱动芯片手册 |
| [MAX98357A 数据手册](https://www.analog.com/media/en/technical-documentation/data-sheets/MAX98357A-MAX98357B.pdf) | I2S D 类功放芯片手册 |

### 工具与辅助学习 | Tools & Supplementary Learning

| 资源 | 说明 |
|------|------|
| [Arduino IDE 下载](https://www.arduino.cc/en/software) | 官方 Arduino 开发环境 |
| [Adafruit ST7789 库文档](https://github.com/adafruit/Adafruit-ST7735-ST7789-Library) | 屏幕驱动库使用说明 |
| [ArduinoWebsockets 库](https://github.com/gilmaimon/ArduinoWebsockets) | WebSocket 客户端库 |
| [SCServo 库](https://github.com/SC-Robotics/SCServo) | STS3032 总线舵机控制库 |
| [LittleFS for ESP32](https://github.com/earlephilhower/arduino-esp32littlefs-plugin) | LittleFS 上传插件 |
| [C++ 基础教程](https://www.runoob.com/cplusplus/cpp-tutorial.html) | C++ 语言基础速查 |
| [Python 基础教程](https://www.runoob.com/python3/python3-tutorial.html) | Python 语言基础速查 |

## 常见问题 | FAQ

**Q: 进阶版和基础版哪个更难？**
A: 两者侧重不同。基础版要求从零焊接 PCB 并使用 ESP-IDF（C 语言，更底层），焊接是主要门槛。进阶版使用成品开发板 + Arduino IDE（更易上手），但多了 Python 后端开发、多模态 AI API 调用、WebSocket 通信和四足步态算法。如果你对 AI 和 Python 更感兴趣，选进阶版；如果你对硬件底层和焊接感兴趣，选基础版。

Which track is harder?
They focus on different skills. The base track requires PCB soldering from scratch and uses ESP-IDF (C, more low-level) -- soldering is the main barrier. The advanced track uses a ready-made dev board + Arduino IDE (easier to start), but adds Python backend development, multimodal AI API calls, WebSocket communication, and quadruped gait algorithms. Choose the advanced track if you are more interested in AI and Python; choose the base track if you prefer hardware fundamentals and soldering.

**Q: 没有 Arduino 基础能做吗？**
A: 可以。Arduino 框架比 ESP-IDF 更易入门，Day 1 会从 Blink 点灯开始。建议提前了解 C/C++ 基本语法（变量、函数、条件、循环）。如果你已经做过 Arduino 项目，会更轻松。

Can I do this without Arduino experience?
Yes. Arduino is easier to learn than ESP-IDF. Day 1 starts from a Blink sketch. We recommend self-studying basic C/C++ syntax beforehand (variables, functions, conditionals, loops). Prior Arduino experience will make things easier.

**Q: Qwen 3.5 Omni API 需要付费吗？**
A: 阿里云 DashScope 提供新用户免费额度，Qwen 模型有免费调用次数。具体额度以阿里云官网为准。Day 10 才涉及 AI 功能，不影响前 9 天的硬件和固件开发。如果暂时不想配置 AI，可以先跳过，只测试硬件功能。

Does the Qwen 3.5 Omni API cost money?
Alibaba Cloud DashScope offers free credits for new users, and Qwen models have free-tier quotas. Check the official site for current limits. AI features start on Day 10 and do not affect hardware/firmware work in Days 1-9. You can skip AI configuration initially and focus on hardware testing.

**Q: STS3032 舵机动作不对或抽搐怎么办？**
A: 首先确认舵机 ID 是否和代码约定一致（ID1=左前腿、ID2=右前腿、ID3=左后腿、ID4=右后腿），检查 TX/RX 是否接反。首次测试时务必空载，确认方向和角度范围正确后再安装到结构上。供电电压要符合舵机要求，电流不够也会导致异常。

What if STS3032 servos behave incorrectly or twitch?
First confirm servo IDs match the code convention (ID1=front-left, ID2=front-right, ID3=rear-left, ID4=rear-right), and check if TX/RX are swapped. Always test without load first. Verify direction and angle range before mounting. Ensure power supply voltage meets servo specs -- insufficient current also causes anomalies.

**Q: ESP32 一动舵机就重启怎么办？**
A: 大概率是舵机供电不足或没有共地。舵机必须单独供电，外部电源 GND 要和 ESP32 GND 接在一起。不要从 ESP32 的 3.3V 或 5V 引脚直接给舵机供电。

What if the ESP32 reboots when servos move?
This is almost certainly insufficient servo power or missing common ground. Servos must have a separate power supply, and the external power GND must be connected to the ESP32 GND. Never power servos directly from the ESP32 3.3V or 5V pins.

**Q: 摄像头没有画面怎么办？**
A: 先检查串口日志确认摄像头初始化是否成功。再看后端终端是否收到 `/ws/camera` 的 WebSocket 连接。确认 ESP32 和电脑在同一个 Wi-Fi 网络下，`SERVER_HOST` 填的是电脑 IP 而不是 ESP32 IP。如果电脑开了防火墙，需要放行 8081 端口。

What if the camera shows no image?
Check the serial log to confirm camera initialization succeeded. Then check the backend terminal for `/ws/camera` WebSocket connections. Ensure the ESP32 and computer are on the same Wi-Fi network, and `SERVER_HOST` is the computer IP, not the ESP32 IP. If a firewall is running, allow port 8081.

## 许可证 | License

本项目基于原始仓库 [AI-FanGe/AI_DesktopCat_Qwen3.5Omni](https://github.com/AI-FanGe/AI_DesktopCat_Qwen3.5Omni) 的开源协议发布，详见原始仓库 LICENSE 文件。

This project is released under the license of the original repository. See the LICENSE file in the original GitHub repository for details.

---

*最后更新：2026-05-27 | Last Updated: 2026-05-27*
