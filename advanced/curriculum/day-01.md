# Day 01: Arduino IDE环境搭建与XIAO ESP32S3入门 | Arduino IDE Setup & XIAO ESP32S3 Introduction

> **今日目标:**
> - 完成Arduino IDE的安装与配置
> - 添加ESP32开发板支持并安装所有必需库
> - 了解XIAO ESP32S3 Sense的引脚分布与硬件特性
> - 完成第一个Blink程序并通过串口监视器调试
>
> **产出:** 配置完成的开发环境，成功运行的Blink程序和串口通信

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间回顾 | 课程概述与学习目标介绍 |
| 09:15 - 10:30 | 知识讲解 | Arduino IDE安装与ESP32开发板配置 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | 安装库文件、配置开发板参数 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | 编写并烧录第一个Blink程序 |
| 15:15 - 16:30 | 拓展练习 | 串口通信练习与PSRAM配置 |
| 16:30 - 17:00 | 总结分享 | 今日收获总结 |

## 上午: Arduino IDE安装与配置 | Morning: Arduino IDE Installation & Configuration

### 1. 下载安装Arduino IDE | Download & Install Arduino IDE

从官方网站下载最新版本的Arduino IDE 2.x：
Download the latest Arduino IDE 2.x from the official website:

- 官网地址: https://www.arduino.cc/en/software
- 支持 Windows / macOS / Linux
- 下载后按照安装向导完成安装

### 2. 添加ESP32开发板支持 | Add ESP32 Board Support

打开Arduino IDE，进入 **文件 → 首选项** (File → Preferences)，在"附加开发板管理器网址"中添加：

Open Arduino IDE, go to **File → Preferences**, add the following URL to "Additional Board Manager URLs":

```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

然后进入 **工具 → 开发板 → 开发板管理器**，搜索 `esp32`，安装 **esp32 by Espressif Systems**（建议版本 2.0.17 或更高）。

Then go to **Tools → Board → Boards Manager**, search for `esp32`, and install **esp32 by Espressif Systems** (recommended version 2.0.17 or later).

### 3. 安装所需库 | Install Required Libraries

进入 **工具 → 管理库** (Tools → Manage Libraries)，搜索并安装以下库：

Go to **Tools → Manage Libraries**, search and install the following:

| 库名 Library | 用途 Purpose |
|-------------|-------------|
| ESP32Servo | ESP32舵机控制 Servo control |
| Adafruit GFX Library | 图形绘制基础 Graphics primitives |
| Adafruit ST7789 | ST7789屏幕驱动 ST7789 display driver |
| Adafruit PWM Servo Driver Library | PCA9685驱动 PCA9685 driver |
| ArduinoWebsockets | WebSocket通信 WebSocket communication |
| JPEGDEC | JPEG图像解码 JPEG image decoding |
| SCServo | 飞特串行总线舵机 Feetech serial servo |

安装时注意选择正确的库（核对作者名），部分库会自动安装依赖项。

Make sure to select the correct library (verify the author name). Some libraries will automatically install dependencies.

### 4. XIAO ESP32S3 Sense 引脚分布 | XIAO ESP32S3 Sense Pinout

XIAO ESP32S3 Sense 是一款紧凑型ESP32-S3开发板，集成了摄像头模块：

The XIAO ESP32S3 Sense is a compact ESP32-S3 board with an integrated camera module:

```
                    XIAO ESP32S3 Sense
                   ┌──────────────────┐
              5V ──┤                  ├── VIN (5V)
             GND ──┤                  ├── GND
           (A0)D0 ─┤                  ├── D10 (A10)
           (A1)D1 ─┤                  ├── D9  (A9)
           (A2)D2 ─┤                  ├── D8  (A8)
           (A3)D3 ─┤   ESP32-S3      ├── D7  (A7)
           (A4)D4 ─┤   240MHz        ├── D6  (A6)
           (A5)D5 ─┤   Dual Core     ├── D5  (A5)  ← 注：左右对称
                   └──────────────────┘
                        USB-C 底部
```

关键参数 | Key Specs:
- **处理器 MCU:** ESP32-S3 双核 240MHz
- **闪存 Flash:** 8MB
- **PSRAM:** 8MB (需在IDE中启用 / Must enable in IDE)
- **摄像头 Camera:** OV2640 (通过CAM接口 / via CAM connector)
- **数字I/O:** 11个 GPIO (D0-D10)
- **模拟输入:** 9个 ADC通道 (A0-A10)
- **接口:** I2C, SPI, UART, I2S
- **无线:** Wi-Fi 802.11 b/g/n, Bluetooth 5.0

### 5. 开发板配置 | Board Configuration

在Arduino IDE中选择正确的开发板和参数：

Select the correct board and settings in Arduino IDE:

```
开发板 Board:        XIAO_ESP32S3
上传速度 Upload Speed: 921600
USB模式 USB Mode:     Hardware CDC and JTAG
分区方案 Partition Scheme: Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)
PSRAM:               OPI PSRAM   ← 必须启用！ / Must enable!
端口 Port:            选择对应的COM口 / Select corresponding COM port
```

## 下午: 第一个程序与调试 | Afternoon: First Program & Debugging

### 6. Blink程序 | Blink Sketch

```cpp
// Day 01 - Blink 程序
// XIAO ESP32S3 内置LED在引脚 D2 (GPIO2) 上
// 注意：部分版本LED可能在GPIO21，请根据实际情况调整

#define LED_PIN 21  // XIAO ESP32S3 内置LED引脚

void setup() {
  // 初始化LED引脚为输出模式
  pinMode(LED_PIN, OUTPUT);

  // 初始化串口通信，波特率115200
  Serial.begin(115200);
  Serial.println("XIAO ESP32S3 Blink 程序启动！");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);   // 点亮LED
  Serial.println("LED ON  💡");
  delay(1000);                   // 等待1秒

  digitalWrite(LED_PIN, LOW);    // 熄灭LED
  Serial.println("LED OFF");
  delay(1000);                   // 等待1秒
}
```

### 7. 串口监视器 | Serial Monitor

烧录程序后，打开 **工具 → 串口监视器** (Tools → Serial Monitor)：

After uploading, open **Tools → Serial Monitor**:

- 设置波特率为 **115200**
- 观察输出信息是否正常打印
- 尝试在发送框中输入文字并点击发送

```cpp
// Day 01 - 串口交互练习
// 通过串口监视器与开发板交互

#define LED_PIN 21

bool ledState = false;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("=== 串口交互测试 ===");
  Serial.println("输入 'on'  点亮LED");
  Serial.println("输入 'off' 熄灭LED");
  Serial.println("输入 'blink' LED闪烁");
  Serial.println("===================");
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();  // 去除首尾空白字符

    if (command == "on") {
      digitalWrite(LED_PIN, HIGH);
      Serial.println(">> LED已点亮");
    }
    else if (command == "off") {
      digitalWrite(LED_PIN, LOW);
      Serial.println(">> LED已熄灭");
    }
    else if (command == "blink") {
      Serial.println(">> LED闪烁3次");
      for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
      }
    }
    else {
      Serial.println(">> 未知命令: " + command);
    }
  }
}
```

### 8. PSRAM启用与验证 | PSRAM Enable & Verify

PSRAM（外部SRAM）对于图像处理和音频缓冲至关重要，必须启用：

PSRAM (external SRAM) is critical for image processing and audio buffering. It must be enabled:

```cpp
// Day 01 - PSRAM 检测程序

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== PSRAM 检测 ===");

  // 检查PSRAM是否可用
  if (psramFound()) {
    Serial.println("PSRAM: 已找到 ✓");
    Serial.printf("PSRAM 大小: %d bytes (%.1f MB)\n",
                  ESP.getPsramSize(),
                  ESP.getPsramSize() / 1024.0 / 1024.0);
    Serial.printf("PSRAM 可用: %d bytes\n", ESP.getFreePsram());

    // 测试PSRAM分配
    uint8_t* psramBuffer = (uint8_t*) ps_malloc(1024 * 100);  // 分配100KB
    if (psramBuffer != NULL) {
      Serial.println("PSRAM 分配测试: 成功 ✓");
      free(psramBuffer);
    } else {
      Serial.println("PSRAM 分配测试: 失败 ✗");
    }
  } else {
    Serial.println("PSRAM: 未找到 ✗");
    Serial.println("请在 工具→PSRAM 中选择 'OPI PSRAM'");
  }

  Serial.printf("Flash 大小: %d bytes (%.1f MB)\n",
                ESP.getFlashChipSize(),
                ESP.getFlashChipSize() / 1024.0 / 1024.0);
}

void loop() {
  // 打印可用PSRAM和堆内存
  static int count = 0;
  if (count % 100 == 0) {  // 每10秒打印一次
    Serial.printf("[Heap: %d, PSRAM free: %d]\n",
                  ESP.getFreeHeap(), ESP.getFreePsram());
  }
  delay(100);
  count++;
}
```

### 9. 分区方案说明 | Partition Scheme Explanation

ESP32的Flash被划分为多个区域，分区方案决定了各区域大小：

The ESP32 Flash is divided into regions. The partition scheme determines their sizes:

```
Default 4MB with spiffs:
┌──────────┬──────────┬──────────┐
│ Bootloader│   APP    │  SPIFFS  │
│  (16KB)   │ (1.2MB)  │ (1.5MB)  │
└──────────┴──────────┴──────────┘

Huge APP 3MB:
┌──────────┬──────────┬──────────┐
│ Bootloader│   APP    │  SPIFFS  │
│  (16KB)   │  (3MB)   │ (几百KB) │
└──────────┴──────────┴──────────┘
```

本项目推荐使用 **"Huge APP (3MB)"** 分区方案，因为固件较大（含摄像头、音频、WebSocket等功能）。

For this project, we recommend the **"Huge APP (3MB)"** partition scheme since the firmware is large (camera, audio, WebSocket features).

## 拓展练习 | Extension Exercises

1. **呼吸灯 Breathing LED**: 使用PWM实现LED呼吸效果

```cpp
#define LED_PIN 21

void setup() {
  // ESP32的LED PWM功能不需要调用pinMode
  // ledcSetup(channel, freq, resolution)
  ledcSetup(0, 5000, 8);       // 通道0, 5kHz, 8位分辨率
  ledcAttachPin(LED_PIN, 0);   // 将引脚绑定到通道0
}

void loop() {
  // 渐亮
  for (int brightness = 0; brightness <= 255; brightness++) {
    ledcWrite(0, brightness);
    delay(10);
  }
  // 渐暗
  for (int brightness = 255; brightness >= 0; brightness--) {
    ledcWrite(0, brightness);
    delay(10);
  }
}
```

2. **GPIO测试 GPIO Test**: 逐个测试所有GPIO引脚的输入输出功能
3. **串口绘图 Serial Plotter**: 使用串口绘图器可视化ADC读取的模拟值

## 今日总结 | Day Summary

- 成功安装Arduino IDE并配置了ESP32开发环境
- 了解了XIAO ESP32S3 Sense的硬件规格和引脚分布
- 安装了项目所需的所有库文件
- 完成了Blink程序和串口通信
- 启用了PSRAM并验证了功能
- 了解了分区方案对项目的影响
- **明日预告**: 硬件认知与舵机控制基础
