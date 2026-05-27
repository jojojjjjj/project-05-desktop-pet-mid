# 项目05：桌宠机器人 | Project 05: Desktop Pet Robot

> 本项目提供**两个平行轨道**，学员可根据基础和兴趣选择：
> - **Track A（基础版）**：ESP-SparkBot，ESP-IDF 框架，从焊接开始构建完整桌面机器人
> - **Track B（进阶版）**：AICat，Arduino 框架 + Qwen 3.5 Omni 多模态云端大模型，打造能听、能看、能说、能走的 AI 桌面猫
>
> This project offers **two parallel tracks**:
> - **Track A (Standard)**: ESP-SparkBot, ESP-IDF framework, build a desktop robot from soldering
> - **Track B (Advanced)**: AICat, Arduino framework + Qwen 3.5 Omni multimodal cloud AI, build an AI desktop cat that hears, sees, speaks, and walks

---

## 轨道对比 | Track Comparison

| 维度 | Track A — 基础版 (ESP-SparkBot) | Track B — 进阶版 (AICat + Qwen 3.5 Omni) |
|------|------|------|
| **定位** | 嵌入式开发入门，从零焊接PCB | 多模态AI硬件集成，云端大模型接入 |
| **主控** | ESP32-S3-WROOM-1-N16R8 | Seeed XIAO ESP32S3 Sense |
| **开发框架** | ESP-IDF (C语言) | Arduino IDE (C/C++) + Python 后端 |
| **AI能力** | 本地语音唤醒 + 云端LLM API | 云端多模态大模型 Qwen 3.5 Omni（语音+文本+视觉） |
| **运动能力** | 静态桌面设备（可选履带底盘） | 4足行走（STS3032总线舵机）+ 表情舵机 |
| **显示** | 1.54寸 240×240 LVGL GUI | 1.83寸 170×320 表情动画系统（LittleFS存储） |
| **硬件技能** | PCB焊接、贴片元件 | 面包板接线、舵机组装、3D打印外壳 |
| **软件技能** | ESP-IDF、FreeRTOS、LVGL | Arduino、Python后端、WebSocket、API集成 |
| **难度** | ⭐⭐⭐ 中等 | ⭐⭐⭐⭐ 进阶 |
| **总成本** | ~120-150元 | ~350-400元 |
| **课程天数** | 12天 | 12天 |
| **开源仓库** | [Gitee](https://gitee.com/esp-friends/esp_sparkbot) | [GitHub](https://github.com/AI-FanGe/AI_DesktopCat_Qwen3.5Omni) |

---

## Track A：ESP-SparkBot 基础版 | Track A: ESP-SparkBot (Standard)

> 基于ESP32-S3的大模型AI桌面机器人 | AI Desktop Robot powered by ESP32-S3

**作者**: 乐鑫小铁匠 (ESP_Friends / Espressif Engineers) | **开源协议**: Apache-2.0

### 项目简介 | Project Overview

你是否想过亲手打造一个能听、能看、能说、能动的AI桌面机器人？ESP-SparkBot 是一个基于 ESP32-S3 的开源硬件项目，它集成了大模型AI对话、语音识别、人脸识别、天气时钟、蓝牙音箱、体感游戏等多种功能，让你从零开始体验嵌入式AI开发的完整流程。

Have you ever dreamed of building your own AI desktop robot that can hear, see, speak, and move? ESP-SparkBot is an open-source hardware project based on the ESP32-S3. It integrates LLM-driven AI conversation, voice recognition, face detection, weather clock, Bluetooth speaker, motion-sensing games, and more -- all built from scratch following a complete embedded AI development workflow.

本项目以嵌入式开发为主线，涵盖 ESP-IDF 开发环境搭建、硬件焊接组装、驱动开发、传感器数据采集、Wi-Fi/蓝牙通信、AI模型部署（语音/视觉）等核心技术。整个项目为期12天，从电烙铁拿起到成品展示，循序渐进地引导你迈入嵌入式AI的世界。

The project focuses on embedded development, covering ESP-IDF toolchain setup, PCB soldering and assembly, driver development, sensor data acquisition, Wi-Fi/Bluetooth communication, and AI model deployment (voice/vision). The 12-day curriculum guides you from picking up a soldering iron to demoing a finished product.

| 项目 | 开源地址 |
|------|----------|
| 硬件设计 | https://oshwhub.com/esp-college/esp-sparkbot |
| 软件源码 | https://gitee.com/esp-friends/esp_sparkbot |
| 3D打印模型 | https://makerworld.com.cn/zh/models/1025233-esp-sparkbot-esp32-s3-da-mo-xing-ai-zhuo-mian-ji-q |
| 交流论坛 | https://www.sparkbot.com.cn/ |

## 最终效果 | Final Result

完成本项目后，你将拥有：
- 一个搭载 1.54寸 LCD 屏幕、摄像头、麦克风、喇叭的桌面机器人
- 支持 DeepSeek / ChatGPT / 文心一言 / 豆包 / Kimi 等大模型的智能语音对话
- 实时天气时钟显示（IP定位自动获取本地天气）
- 蓝牙音箱功能（通过小度APP配网，支持QQ音乐、新闻、有声读物）
- 赛博木鱼：触摸感应敲木鱼，支持ESP-NOW多设备同步
- 赛博骰子：利用BMI270加速度传感器模拟3D骰子效果
- 2048游戏：通过手势操控的经典益智游戏
- 语音识别 & 人脸识别 & 移动检测（基于ESP-SR和ESP-WHO库）
- USB投屏副屏 + 双向音频传输
- 可选：连接履带底盘变身遥控小车（支持Web控制界面 + 无线图传）

After completing this project, you will have:
- A desktop robot with a 1.54" LCD, camera, microphone, and speaker
- Intelligent voice conversation powered by DeepSeek / ChatGPT / Wenxin Yiyan / Doubao / Kimi / ZhipuAI / Mistral AI / XiaoZhi AI and more
- Real-time weather & clock display (auto-location via IP geolocation)
- Bluetooth speaker mode (configured via Xiaodu app, supports QQ Music, news, audiobooks)
- Cyber Muyu: touch-sensitive wooden fish tapping with ESP-NOW multi-device sync
- Cyber Dice: BMI270 accelerometer-based 3D dice simulation
- 2048 Game: gesture-controlled classic puzzle game
- Voice recognition & face detection & motion detection (powered by ESP-SR & ESP-WHO)
- USB screen casting as a secondary display with bidirectional audio
- Optional: tracked chassis for remote-control car mode (Web control + wireless video streaming)

## 核心技术 | Core Technologies

| 技术 | 用途 | 说明 |
|------|------|------|
| ESP32-S3-WROOM-1-N16R8 | 主控芯片 | 双核Xtensa LX7 @ 240MHz, 16MB Flash + 8MB PSRAM |
| ESP-IDF (v5.3/v5.4) | 开发框架 | Espressif 官方物联网开发框架 |
| ST7789 (SPI LCD) | 显示驱动 | 1.54寸 240×240 彩色TFT屏幕 |
| OV2640 (DVP接口) | 图像采集 | 200万像素摄像头，用于人脸识别与移动检测 |
| BMI270 | 惯性测量 | 6轴陀螺仪/加速度计，用于体感交互 |
| ES8311 + NS4150B | 音频系统 | 音频编解码器 + 3W D类功放，支持录音与播放 |
| ESP-SR | 语音识别 | Espressif 轻量级语音识别框架 |
| ESP-WHO | 人脸识别 | Espressif 轻量级人脸检测与识别框架 |
| Wi-Fi 6 + BLE 5.0 | 无线通信 | 网络连接 + 蓝牙配网 + ESP-NOW 设备间通信 |
| USB Device Mode | USB投屏 | 模拟USB显示器 + UAC音频设备 |
| LVGL | GUI框架 | 轻量级嵌入式图形库，用于界面渲染 |
| ESP-NOW | 多机通信 | 低延迟点对点协议，用于赛博木鱼等多设备同步 |

## 硬件清单 | Hardware List

### 主控与核心元件 | Main Controller & Core Components

| 部件 | 规格 | 价格 | 说明 |
|------|------|------|------|
| 主控模组 | ESP32-S3-WROOM-1-N16R8 | ~25元 | 16MB Flash + 8MB PSRAM |
| LCD屏幕 | 1.54寸 SPI, 240×240, ST7789 | ~18元 | IPS全视角 |
| 摄像头 | OV2640, DVP接口, 200万像素 | ~12元 | 含FPC排线 |
| 陀螺仪/加速度计 | BMI270 | ~8元 | 6轴IMU传感器 |
| 音频编解码器 | ES8311 | ~5元 | I2S接口 |
| 音频功放 | NS4150B | ~3元 | 3W D类功放 |
| 喇叭 | 4Ω/3W | ~5元 | 小尺寸动圈喇叭 |
| 麦克风 | 驻极体麦克风 | ~2元 | 模拟麦克风 |

### 电源与PCB | Power & PCB

| 部件 | 规格 | 价格 | 说明 |
|------|------|------|------|
| 锂电池 | 3.7V 聚合物锂电池, 2000mAh | ~15元 | 502540尺寸 |
| PCB打板 | 3块PCB（1.27mm排针排母堆叠） | ~30元 | 嘉立创打样, 板厚1.0mm |
| 磁吸接口 | 4P (5V/GPIO48/GPIO38/GND) | ~3元 | 用于连接履带底盘 |
| 排针排母 | 1.27mm间距 | ~2元 | PCB堆叠连接 |

### 外壳 | Enclosure

| 部件 | 规格 | 价格 | 说明 |
|------|------|------|------|
| 主体后壳 | 3D打印 | ~15元 | PLA/PETG材质 |
| 前盖(带屏幕卡口) | 3D打印 | ~10元 | 含屏幕固定结构 |
| 摄像头前盖+后盖 | 3D打印 | ~5元 | 摄像头模块外壳 |

### 工具清单 | Tools Required

| 工具 | 用途 | 说明 |
|------|------|------|
| 电烙铁 + 焊锡丝 | 焊接元件 | 推荐936/TS100/T12焊台 |
| 热风枪 + 锡浆 | 焊接QFN/BGA | 回流焊用，可选 |
| 水口钳 | 剪引脚 | 修剪排针过长的引脚 |
| 镊子 | 元件夹取 | 防静电镊子更佳 |
| 万用表 | 电路检测 | 排查焊接短路/断路 |
| 助焊膏 + 洗板水 | 焊接辅助 | 提升焊接质量 |
| B-7000胶水 | 外壳粘接 | 固定喇叭、磁吸接口 |
| PCB夹 / 焊接台 | 固定PCB | 辅助焊接操作 |

**总成本约120-150元**（不含电烙铁等可复用工具），远低于500元预算。

**Total cost approx. 120-150 CNY** (excluding reusable tools like soldering iron), well under the 500 CNY budget.

## 软件环境 | Software Environment

| 软件 | 版本 | 用途 |
|------|------|------|
| ESP-IDF | v5.3 / v5.4 | 乐鑫官方物联网开发框架（编译、烧录、调试） |
| VS Code | latest | 代码编辑器，搭配 ESP-IDF 插件 |
| ESP-IDF VS Code Extension | latest | 一键编译、烧录、监控串口输出 |
| Python | 3.8+ | ESP-IDF 依赖及部分辅助脚本 |
| Git | latest | 版本控制与代码克隆 |
| CH340/CP210x 驱动 | latest | USB串口驱动（取决于开发板） |
| 小度APP | latest | 蓝牙配网工具（蓝牙音箱功能） |
| 立创EDA | 在线版 | PCB电路图查看与修改（可选） |
| 切片软件 | Cura / Bambu Studio | 3D打印切片（如需自己打印外壳） |

## 快速开始 | Quick Start

### Step 1: 克隆项目 | Clone the Project

```bash
# 克隆软件仓库
git clone https://gitee.com/esp-friends/esp_sparkbot.git
cd esp_sparkbot

# 初始化子模块（如有）
git submodule update --init --recursive
```

### Step 2: 安装 ESP-IDF | Install ESP-IDF

```bash
# Windows: 使用官方离线安装器（推荐）
# 下载地址: https://dl.espressif.com/dl/esp-idf/
# 选择 ESP-IDF v5.3 或 v5.4 版本

# macOS / Linux: 使用命令行安装
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout release/v5.3
./install.sh esp32s3
. ./export.sh
```

### Step 3: 编译与烧录 | Build & Flash

```bash
# 进入工厂综合例程
cd examples/factory_demo

# 设置目标芯片
idf.py set-target esp32s3

# 配置项目（设置Wi-Fi、LLM API Key等）
idf.py menuconfig

# 编译
idf.py build

# 烧录（将ESP32-S3通过USB连接到电脑）
idf.py -p COM3 flash        # Windows
# idf.py -p /dev/ttyUSB0 flash   # Linux
# idf.py -p /dev/cu.usbserial-* flash  # macOS

# 查看串口输出
idf.py -p COM3 monitor
```

### Step 4: 硬件组装 | Hardware Assembly

1. 焊接PCB（详见 `hardware/assembly-steps.md`）
2. 安装3D打印外壳
3. 连接电池、喇叭、摄像头
4. 上电测试

### Step 5: 配网与配置 | Network & Configuration

1. 通过小度APP进行蓝牙配网
2. 在 `menuconfig` 中配置大模型API Key（DeepSeek / ChatGPT 等）
3. 重启设备，检查天气时钟是否正常显示

## 课程安排 | Course Schedule

### 第一阶段：嵌入式开发环境搭建与硬件组装（Day 1-4）
Phase 1: Embedded Environment & Hardware Assembly

| 天数 | 主题 | 核心知识点 |
|------|------|-----------|
| Day 1 | ESP32-S3平台入门与环境搭建 | ESP-IDF安装、VS Code配置、Hello World烧录、串口监视器使用 |
| Day 2 | 硬件原理图阅读与元器件认知 | 电路原理图阅读、贴片元件识别、万用表使用、BOM核对 |
| Day 3 | PCB焊接实践（上） | 电烙铁使用、贴片阻容焊接、QFN封装焊接技巧、助焊膏使用 |
| Day 4 | PCB焊接实践（下）与整机组装 | 排针排母堆叠焊接、外壳组装、电池连接、首次上电测试 |

### 第二阶段：核心功能开发（Day 5-8）
Phase 2: Core Function Development

| 天数 | 主题 | 核心知识点 |
|------|------|-----------|
| Day 5 | LCD驱动与LVGL图形界面 | SPI通信协议、ST7789驱动、LVGL基础控件、界面布局与事件回调 |
| Day 6 | 音频系统与蓝牙音箱 | I2S音频接口、ES8311驱动、A2DP蓝牙音频、小度APP配网 |
| Day 7 | 传感器驱动与体感交互 | I2C总线、BMI270数据读取、姿态解算、赛博骰子/2048游戏实现 |
| Day 8 | Wi-Fi网络与天气时钟 | HTTP客户端、JSON解析、IP定位API、天气API调用、NTP时间同步 |

### 第三阶段：AI功能集成与项目展示（Day 9-12）
Phase 3: AI Integration & Project Demo

| 天数 | 主题 | 核心知识点 |
|------|------|-----------|
| Day 9 | 语音识别与人脸识别 | ESP-SR语音命令识别、ESP-WHO人脸检测、OV2640图像采集、移动检测 |
| Day 10 | 大模型AI对话集成 | WebSocket/HTTP API调用、多模型适配（DeepSeek/ChatGPT/豆包等）、流式对话、TTS语音合成 |
| Day 11 | USB投屏与扩展功能 | USB Device模式、UVC/UAC协议、履带底盘运动控制、ESP-NOW多机通信 |
| Day 12 | 项目展示与总结 | 功能演示、技术复盘、Bug修复经验分享、拓展方向讨论、Demo网站搭建 |

## 评分标准 | Grading Rubric

| 维度 | 权重 | 说明 |
|------|------|------|
| 技术实现 | 40% | 功能完整度、代码质量、硬件焊接质量、Bug修复能力 |
| 文档质量 | 15% | README、代码注释、硬件接线记录、故障排除日志 |
| 演示展示 | 15% | Final Demo清晰度、表达能力、Q&A应对 |
| Demo网站 | 15% | 项目展示网站的可访问性、内容完整性、视觉呈现 |
| 进度汇报 | 10% | 每日Check-in质量、问题分析深度、解决方案记录 |
| 创新拓展 | 10% | 额外功能、个性化外壳设计、自定义UI主题、创意交互方式 |

## 项目结构 | Project Structure

```
esp_sparkbot/
├── README.md                               # 项目说明文档
├── LICENSE                                  # Apache-2.0 许可证
├── components/                              # ESP-IDF 组件库
│   ├── lvgl/                               # LVGL 图形库组件
│   ├── esp-sr/                             # ESP-SR 语音识别组件
│   ├── esp-who/                            # ESP-WHO 人脸识别组件
│   ├── st7789/                             # ST7789 LCD驱动组件
│   ├── es8311/                             # ES8311 音频编解码驱动
│   ├── bmi270/                             # BMI270 传感器驱动
│   ├── ov2640/                             # OV2640 摄像头驱动
│   └── esp-now/                            # ESP-NOW 通信组件
├── main/                                    # 主应用代码
│   ├── CMakeLists.txt                      # ESP-IDF 构建配置
│   ├── main.c                              # 主入口
│   ├── app_audio.c / .h                   # 音频模块
│   ├── app_display.c / .h                 # 显示模块
│   ├── app_sensor.c / .h                  # 传感器模块
│   ├── app_network.c / .h                 # 网络模块
│   ├── app_ai.c / .h                      # AI对话模块
│   ├── app_game.c / .h                    # 游戏模块（木鱼/骰子/2048）
│   └── app_usb.c / .h                     # USB投屏模块
├── examples/                                # 独立功能例程
│   ├── factory_demo/                       # 综合工厂演示程序
│   ├── bilibili_fans/                      # B站粉丝显示器
│   ├── tank/c2_tracked_chassis/            # 履带底盘驱动
│   ├── tank/sparkbot_motion_control/       # 运动控制例程
│   ├── usb_extend_screen_s3/               # USB副屏例程
│   └── voice_control_car/                  # 语音控制小车
├── hardware/                                # 硬件设计文件
│   ├── schematic/                           # 原理图（PDF + 立创EDA源文件）
│   ├── pcb/                                 # PCB Gerber文件
│   ├── BOM.xlsx                             # 物料清单
│   ├── 3d-models/                           # 3D打印STL模型文件
│   └── assembly-guide.md                   # 组装图文指南
├── curriculum/                               # 课程文档（本夏令营定制）
│   ├── overview.md                          # 课程总览
│   ├── prerequisites.md                     # 前置知识清单
│   ├── assignments.md                       # 作业说明
│   ├── grading-rubric.md                    # 详细评分标准
│   └── day-01.md ~ day-12.md               # 每日课程教案
├── assignments/                              # 作业与考核
│   ├── day-01-checkin.md                    # Day 1进度汇报
│   ├── week-1-checkin.md                    # 第一周进度汇总
│   ├── week-2-checkin.md                    # 第二周进度汇总
│   ├── final-presentation.md                # 最终展示要求
│   └── rubric.md                            # 详细评分表
└── docs/                                     # 补充文档
    ├── api-reference.md                     # API参考
    ├── troubleshooting.md                   # 故障排除指南
    └── faq.md                               # 常见问题
```

## 学习资源 | Learning Resources

### B站视频教程 | Bilibili Video Tutorials

| 视频 | UP主 | 推荐理由 |
|------|------|----------|
| [ESP-SparkBot 开源项目介绍](https://space.bilibili.com/521895131) | 乐鑫小铁匠 | 项目作者官方介绍与教程 |
| [ESP32-S3入门教程（保姆级）](https://www.bilibili.com/video/BV1eM4y1y7cV/) | 乐鑫信息科技 | ESP32-S3零基础入门首选 |
| [ESP-IDF开发环境搭建](https://www.bilibili.com/video/BV1vG41187Uo/) | 乐鑫信息科技 | 官方IDF环境搭建指南 |
| [LVGL图形库入门](https://www.bilibili.com/video/BV1sN4y1V7tU/) | 正点原子 | LVGL嵌入式GUI开发 |
| [ESP32 蓝牙音箱DIY](https://search.bilibili.com/all?keyword=ESP32+蓝牙音箱) | 各路UP主 | 蓝牙音频项目参考合集 |
| [ESP32-S3 人脸识别](https://search.bilibili.com/all?keyword=ESP32-S3+人脸识别) | 各路UP主 | ESP-WHO人脸识别应用案例 |
| [PCB焊接教程（手把手）](https://search.bilibili.com/all?keyword=PCB+焊接+教程+新手) | 各路UP主 | 焊接基本功训练 |

### 官方文档与开源仓库 | Official Docs & Open Source

| 资源 | 说明 |
|------|------|
| [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/) | ESP32-S3官方开发文档（中文） |
| [ESP-SR 语音识别文档](https://github.com/espressif/esp-sr) | Espressif 语音识别框架 |
| [ESP-WHO 人脸识别文档](https://github.com/espressif/esp-who) | Espressif 人脸检测与识别框架 |
| [LVGL 官方文档](https://docs.lvgl.io/) | 轻量级嵌入式GUI框架文档 |
| [ESP-SparkBot 硬件开源](https://oshwhub.com/esp-college/esp-sparkbot) | 立创开源硬件平台 - 原理图+PCB |
| [ESP-SparkBot 软件开源](https://gitee.com/esp-friends/esp_sparkbot) | Gitee 代码仓库 |
| [3D打印模型下载](https://makerworld.com.cn/zh/models/1025233-esp-sparkbot-esp32-s3-da-mo-xing-ai-zhuo-mian-ji-q) | MakerWorld 3D模型STL文件 |
| [SparkBot 交流论坛](https://www.sparkbot.com.cn/) | 复刻问题讨论、经验分享 |

### 工具与辅助学习 | Tools & Supplementary Learning

| 资源 | 说明 |
|------|------|
| [立创EDA](https://lceda.cn/) | 在线PCB设计工具，可查看/编辑SparkBot原理图 |
| [Wokwi ESP32模拟器](https://wokwi.com/esp32) | 在线ESP32仿真，无需硬件即可测试代码逻辑 |
| [PlatformIO](https://platformio.org/) | 替代IDE选择（ESP-IDF插件更推荐） |
| [C语言教程（菜鸟教程）](https://www.runoob.com/cprogramming/c-tutorial.html) | C语言基础速查 |
| [Git入门教程](https://www.runoob.com/git/git-tutorial.html) | Git版本控制基础 |
| [嘉立创下单平台](https://www.jlc.com/) | PCB打板与SMT贴片服务 |
| [DeepSeek API文档](https://platform.deepseek.com/api-docs/) | 大模型AI对话接口文档 |
| [OpenAI API文档](https://platform.openai.com/docs/) | ChatGPT API接口文档 |

## 常见问题 | FAQ

**Q: 我没有嵌入式开发基础，能做这个项目吗？**
A: 可以！Day 1会从最基础的ESP-IDF环境搭建开始。但强烈建议提前自学C语言基础（变量、循环、函数、指针），参考菜鸟教程的C语言教程。有Arduino基础的学员会更容易上手。

I have no embedded development background. Can I still do this project?
Yes. Day 1 starts from the very basics of setting up ESP-IDF. However, it is strongly recommended to self-study C basics (variables, loops, functions, pointers) beforehand. Students with Arduino experience will find it easier.

**Q: 焊接很难吗？我从来没碰过电烙铁。**
A: Day 3-4会从零开始教焊接。SparkBot使用的贴片元件以0805/0603封装为主，配合助焊膏和正确手法，新手也能焊好。关键是耐心和细心 -- 我们准备了详细的焊接教学视频和图解。如果实在焊不上，可以请有经验的同学或助教帮忙。

Is soldering hard? I've never used a soldering iron.
Days 3-4 teach soldering from scratch. SparkBot mainly uses 0805/0603 SMD components. With flux paste and proper technique, even beginners can succeed. The key is patience and care -- we provide detailed soldering tutorials. If you really struggle, ask experienced classmates or TAs for help.

**Q: 我的电脑是什么系统？能做吗？**
A: ESP-IDF 支持 Windows 10/11、macOS 和 Linux。Windows用户推荐使用官方离线安装器，Mac/Linux用户使用命令行安装。所有平台功能一致。

What OS do I need?
ESP-IDF supports Windows 10/11, macOS, and Linux. Windows users should use the official offline installer. Mac/Linux users can install via command line. All platforms have equal functionality.

**Q: AI对话功能需要付费吗？**
A: DeepSeek API提供新用户免费额度，豆包（字节跳动）也有免费tier。openai新用户赠送$5额度，国内的大模型API（文心一言、智谱AI）同样有免费额度。如果预算有限，可以只配置免费API。AI功能在Day 10才涉及，不影响前9天的学习。

Does the AI chat feature cost money?
DeepSeek API offers free credits for new users. Doubao (ByteDance) also has a free tier. OpenAI gives new users $5 credits. Domestic Chinese LLM APIs (Wenxin Yiyan, ZhipuAI) also provide free quotas. If budget is tight, stick to free APIs. The AI module is on Day 10 and does not affect earlier learning.

**Q: PCB打板要多久？需要提前准备吗？**
A: 嘉立创PCB打样通常3-5天即可收到。建议开营前一周统一提交打板 + 采购元器件，确保Day 1就能拿到所有物料。3D打印外壳同样需要提前准备（约3-5天），或使用嘉立创的3D打印服务。

How long does PCB fabrication take? Should I prepare in advance?
JLCPCB typically delivers within 3-5 days. We recommend submitting the PCB order and component purchases one week before camp starts. 3D-printed enclosures also require advance preparation (about 3-5 days), or you can use JLCPCB's 3D printing service.

**Q: 为什么选择ESP32-S3而不是树莓派？**
A: ESP32-S3成本低（主控模组仅~25元）、功耗低、集成度高（Wi-Fi+BLE+AI加速），是学习嵌入式AI开发的最佳入门平台。树莓派运行Linux，学习的是应用层开发，ESP32则让你真正理解"从寄存器到应用"的完整链路。

Why ESP32-S3 instead of Raspberry Pi?
The ESP32-S3 is low-cost (module ~25 CNY), low-power, and highly integrated (Wi-Fi+BLE+AI acceleration) -- ideal for learning embedded AI. Raspberry Pi runs Linux and teaches application-layer development. ESP32 lets you truly understand the complete chain from registers to applications.

**Q: 烧录程序后屏幕不亮/没反应怎么办？**
A: 常见原因排查：1) 检查USB线是否为数据线（非纯充电线）；2) 确认烧录时按住BOOT键；3) 检查电池是否连接正确且电压正常；4) 用万用表检查焊接是否有短路或虚焊。详见 `hardware/troubleshooting.md`。

What if the screen stays blank after flashing?
Common causes: 1) Check if USB cable supports data (not charge-only); 2) Ensure you held the BOOT button during flashing; 3) Verify battery connection and voltage; 4) Use a multimeter to check for solder shorts or cold joints. See `hardware/troubleshooting.md` for details.

---

## Track B：AICat 进阶版（多模态大模型）| Track B: AICat Advanced (Multimodal AI)

> 📂 详细内容见 [`advanced/`](advanced/README.md) 目录 | See [`advanced/`](advanced/README.md) for full details

AICat 进阶版在基础版桌宠概念上引入**云端多模态大模型**，让你的桌面猫不仅能做表情和动作，还能通过**语音+视觉**与 Qwen 3.5 Omni 大模型进行多模态对话。

AICat Advanced adds **cloud-based multimodal AI** to the desktop pet concept. Your desktop cat can not only display expressions and move, but also engage in multimodal dialogue (voice + vision) with the Qwen 3.5 Omni model.

### 核心亮点 | Key Highlights

- **多模态AI对话**: 语音输入 → ASR → Qwen 3.5 Omni → TTS → 语音回复，支持语音+文本+视觉多模态输入
- **4足行走**: STS3032总线舵机驱动四条腿，支持走路、坐下、跳跃等动作
- **表情动画系统**: LittleFS存储JPEG帧，屏幕实时播放表情动画 + 嘴型同步
- **Python后端**: Flask/FastAPI服务器处理语音识别、AI对话、TTS合成
- **Web控制面板**: 浏览器实时查看摄像头画面、控制舵机、选择表情

### 快速导航 | Quick Navigation

| 文档 | 说明 |
|------|------|
| [`advanced/README.md`](advanced/README.md) | 进阶版项目完整说明 |
| [`advanced/curriculum/overview.md`](advanced/curriculum/overview.md) | 12天课程总览 |
| [`advanced/curriculum/prerequisites.md`](advanced/curriculum/prerequisites.md) | 前置知识要求 |
| [`advanced/hardware/BOM.md`](advanced/hardware/BOM.md) | 物料清单（~350-400元） |
| [`advanced/hardware/wiring-guide.md`](advanced/hardware/wiring-guide.md) | 接线指南 |
| [`advanced/software/SETUP_GUIDE.md`](advanced/software/SETUP_GUIDE.md) | 软件环境搭建 |
| [`advanced/curriculum/grading-rubric.md`](advanced/curriculum/grading-rubric.md) | 评分标准 |

---

## 许可证 | License

本项目基于 [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0) 开源协议。

This project is licensed under the Apache License 2.0.

```
Copyright 2024 Espressif Systems (Shanghai) Co., Ltd.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

---

*最后更新：2026-05-27 | Last Updated: 2026-05-27*
