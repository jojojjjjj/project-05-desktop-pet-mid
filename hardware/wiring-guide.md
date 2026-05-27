# 接线指南 | Wiring Guide

## 概述 | Overview

ESP-SparkBot 采用 **3 PCB 架构**：核心板 + 底板 + 履带底盘。核心板与底板通过 1.27mm 排针排母堆叠连接；底盘通过底部 4P 磁吸接口连接。本文档详细说明各板的引脚定义和信号连接关系。

ESP-SparkBot uses a **3-PCB architecture**: main board + bottom board + tracked chassis. The main and bottom boards stack via 1.27mm pin headers. The chassis connects via a 4-pin magnetic pogo-pin interface. This document details all pin assignments and signal connections.

---

## 系统连接总览 | System Connection Overview

```
┌──────────────────────────────────────────────────┐
│                  核心板 (Main Board)              │
│                                                  │
│  ┌──────────┐  ┌──────────┐  ┌───────────────┐  │
│  │ ST7789   │  │ OV2640   │  │ ES8311+NS4150B│  │
│  │ 显示屏    │  │ 摄像头    │  │ 音频+功放      │  │
│  │ (SPI)    │  │ (DVP)    │  │ (I2S)         │  │
│  └────┬─────┘  └────┬─────┘  └───────┬───────┘  │
│       │             │               │           │
│  ┌────┴─────────────┴───────────────┴─────────┐  │
│  │        ESP32-S3-WROOM-1-N16R8              │  │
│  └────────────────┬───────────────────────────┘  │
│                   │ 1.27mm 排针 (向下连接底板)     │
└───────────────────┼──────────────────────────────┘
                    │
┌───────────────────┼──────────────────────────────┐
│                   │     底板 (Bottom Board)       │
│  ┌────────────────┴───────────────────────────┐  │
│  │  BMI270 (I2C)  │ USB-C │ 充电IC │ DC-DC   │  │
│  │  触摸FPC (GPIO)│       │        │ 5V→3.3V │  │
│  └────────────────┬───────────────────────────┘  │
│                   │ 4P 磁吸接口 (底部)             │
└───────────────────┼──────────────────────────────┘
                    │
┌───────────────────┼──────────────────────────────┐
│                   │   履带底盘 (Tracked Chassis)   │
│  ┌────────────────┴───────────────────────────┐  │
│  │  ESP8684-WROOM-02C-N2 (独立主控)           │  │
│  │  N20 电机 ×2 | 前灯板 | 后灯板              │  │
│  └────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────┘
```

---

## 核心板引脚定义 | Main Board Pin Assignments

核心板以 ESP32-S3-WROOM-1-N16R8 为主控，所有外设直接连接到模组引脚。

### 显示屏 — ST7789 (SPI)

| ST7789 引脚 | ESP32-S3 引脚 | 信号说明 |
|-------------|---------------|----------|
| SCL (SCK) | GPIOxx (标准 SPI CLK) | SPI 时钟 (Clock) |
| SDA (MOSI) | GPIOxx (标准 SPI MOSI) | SPI 主机输出 (Master Out Slave In) |
| RES (RST) | GPIOxx | 复位信号 (Reset, 低有效) |
| DC (RS) | GPIOxx | 数据/命令选择 (Data/Command) |
| CS | GPIOxx | 片选 (Chip Select, 低有效) |
| BLK (BL) | GPIOxx | 背光控制 (Backlight, PWM) |
| VCC | 3.3V | 电源 |
| GND | GND | 地 |

> **注意**：SPI 通信模式必须设为 **SPI_MODE0**（CPOL=0, CPHA=0）。具体的 GPIO 引脚号请以开源原理图为准，不同版本 PCB 可能有调整。

### 摄像头 — OV2640 (DVP 并行接口)

OV2640 通过 FPC 排线直接连接到核心板上的 FPC 座，使用 8-bit DVP 并行接口：

| 信号组 | 信号线 | 说明 |
|--------|--------|------|
| 数据总线 | D0-D7 | 8-bit 并行像素数据 |
| 同步信号 | PCLK (像素时钟) | 每个 PCLK 传输一个像素 |
| | HREF (行同步) | 有效像素行指示 |
| | VSYNC (帧同步) | 每帧开始标志 |
| 控制信号 | XCLK (主时钟) | 由 ESP32-S3 提供 (通常 20MHz) |
| | PWDN (掉电) | 低功耗模式控制 |
| | RESET | 摄像头复位 |
| I2C 配置 | SIOD (SDA) | SCCB 数据 (兼容 I2C) |
| | SIOC (SCL) | SCCB 时钟 (兼容 I2C) |
| 电源 | 3.3V / 1.8V (内部 LDO) | 核心板提供 |

> **摄像头方向**：OV2640 模组的镜头朝向与排线弯折方向有关。如果在固件中发现画面倒置或镜像，修改 `BSP_CAMERA_VFLIP` 和 `BSP_CAMERA_HMIRROR` 宏即可，不需要重新焊接。

### 音频编解码 — ES8311 (I2S)

ES8311 是 I2S 接口的音频编解码芯片，同时处理录音 (ADC) 和播放 (DAC)：

| ES8311 引脚 | ESP32-S3 引脚 | 信号说明 |
|-------------|---------------|----------|
| I2S_BCLK | GPIOxx (标准 I2S BCLK) | I2S 位时钟 (Bit Clock) |
| I2S_LRCK | GPIOxx (标准 I2S WS) | I2S 左右声道时钟 (LR Clock / Word Select) |
| I2S_ADC_DAT | GPIOxx (标准 I2S DI) | I2S ADC 数据输入 (麦克风 → ESP32) |
| I2S_DAC_DAT | GPIOxx (标准 I2S DO) | I2S DAC 数据输出 (ESP32 → 扬声器) |
| I2C_SCL | GPIOxx | I2C 配置接口时钟 |
| I2C_SDA | GPIOxx | I2C 配置接口数据 |
| MCLK | GPIOxx | 主时钟 (Master Clock, 通常 12.288MHz) |

### 功放 — NS4150B

NS4150B 是 3W D 类音频功放，接在 ES8311 的 DAC 输出之后：

| NS4150B 引脚 | 连接 | 说明 |
|--------------|------|------|
| IN+ / IN- | ES8311 DAC 差分输出 | 差分音频输入 |
| OUT+ / OUT- | 3020 方形腔体喇叭 | 差分音频输出 |
| SD (Shutdown) | GPIOxx | 关断控制 (高有效, 静音) |
| VCC | 5V (USB 供电) | 功放电源 |
| GND | GND | 地 |

### 麦克风 — B4013AM422-42

驻极体电容咪头直接连接到 ES8311 的 MIC 输入引脚：

| 麦克风引脚 | 连接 | 说明 |
|-----------|------|------|
| + (正极) | ES8311 MICIN+ | 通过隔直电容连接 |
| - (负极) | ES8311 MICIN- 或 GND | 差分或单端输入 |

---

## 底板引脚定义 | Bottom Board Pin Assignments

底板通过 1.27mm 排针排母从核心板获取供电和信号，额外搭载以下外设：

### IMU — BMI270 (I2C)

BMI270 通过 I2C 总线连接到 ESP32-S3：

| BMI270 引脚 | 连接 | 说明 |
|-------------|------|------|
| SDA | ESP32-S3 GPIOxx (I2C SDA) | I2C 数据线 |
| SCL | ESP32-S3 GPIOxx (I2C SCL) | I2C 时钟线 |
| INT1 / INT2 | ESP32-S3 GPIOxx | 中断输出 (可选) |
| VDDIO | 1.8V 或 3.3V (核心板提供) | I/O 电压 |
| VDD | 3.3V | 核心电压 |
| GND | GND | 地 |

> I2C 地址：BMI270 默认 7-bit 地址为 **0x68** (AD0=GND) 或 **0x69** (AD0=VDDIO)。上电后用 I2C 扫描确认。

### 触摸按键 — FPC 触摸

| 触摸区域 | ESP32-S3 GPIO | 说明 |
|----------|---------------|------|
| 顶部触摸 | GPIOxx | 头部触摸 |
| 左侧触摸 | GPIOxx | 左侧触摸 |
| 右侧触摸 | GPIOxx | 右侧触摸 |

> 触摸按键使用 ESP32-S3 内置的电容触摸传感器 (Touch Sensor)，不是外挂 IC。灵敏度可通过固件参数调整。

### USB-C 接口 | Type-C Interface

| USB-C 引脚 (CC 逻辑选通后) | 连接 | 说明 |
|----------------------------|------|------|
| VBUS (5V) | 充电 IC 输入 + DC-DC 输入 | USB 5V 供电 |
| CC1 / CC2 | 5.1kΩ 下拉到 GND | Type-C 协议识别 |
| D+ / D- | ESP32-S3 USB OTG (GPIO20/19) | USB 数据传输 + 烧录 |
| GND | GND | 地 |

### 充电管理 | Charging IC

| 信号 | 说明 |
|------|------|
| VBUS_IN (5V) | USB-C VBUS 输入 |
| BAT+ | 电池正极 (3.7V 聚合物锂电池) |
| BAT- | 电池负极 (GND) |
| CHG_STAT | 充电状态指示 LED (可选) |
| TS | 温度检测 (通常接地或 10kΩ NTC) |

### 1.27mm 排针排母接口 | 1.27mm Stacking Header Interface

核心板与底板之间通过 1.27mm 间距排针排母对插。包含以下信号组：

| 信号类别 | 包含信号 | 说明 |
|----------|----------|------|
| 电源 | 5V, 3.3V, GND | 核心板 ↔ 底板互供 |
| I2C | SDA, SCL | 用于底板上的 BMI270 等 I2C 设备 |
| SPI | MOSI, MISO, CLK, CS | 预留，可能用于底板 SPI 设备 |
| GPIO | 若干 | 触摸、中断等信号 |
| UART | TX, RX | 与底盘通信 |

---

## 4P 磁吸接口 | 4-Pin Magnetic Pogo-Pin Interface

底部的 4P 磁吸接口用于连接履带底盘（或未来其他扩展模块）。引脚定义如下：

| Pin | 信号 | 方向 | 说明 |
|-----|------|------|------|
| 1 | **5V** | 输出 (核心板 → 底盘) | 供电输出，也可反向输入为机器人充电 |
| 2 | **GPIO48** (UART TX) | 输出 (核心板 → 底盘) | 串口发送，用于发送运动指令 |
| 3 | **GPIO38** (UART RX) | 输入 (底盘 → 核心板) | 串口接收，用于接收底盘状态反馈 |
| 4 | **GND** | — | 电源 + 信号公共地 |

```
4P 磁吸接口示意图 (俯视)：
┌─────┐
│ 1  2 │  Pin 1: 5V    Pin 2: TX (GPIO48)
│ 3  4 │  Pin 3: RX (GPIO38)  Pin 4: GND
└─────┘
```

> **注意**：磁吸接口没有防反插设计。吸附时凭手感确定方向，确认 4 个 pins 一一对应。错误方向的接触不会立即烧板（因为有 GND 保护），但会导致通信失败。

---

## 履带底盘接线 | Tracked Chassis Wiring

履带底盘由 4 块独立 PCB 组成，通过飞线相互连接。

### 底盘主控板 — ESP8684

| 信号 | ESP8684 引脚 | 目标 | 说明 |
|------|-------------|------|------|
| UART RX | GPIOxx | 磁吸板 Pin 3 (GPIO38) | 接收运动指令 |
| UART TX | GPIOxx | 磁吸板 Pin 2 (GPIO48) | 发送状态回传 |
| MOTOR_L_IN1/IN2 | GPIOxx | 左电机驱动 | 左电机正反转控制 |
| MOTOR_R_IN1/IN2 | GPIOxx | 右电机驱动 | 右电机正反转控制 |
| MOTOR_L_PWM | GPIOxx | 左电机驱动 | 左电机 PWM 调速 |
| MOTOR_R_PWM | GPIOxx | 右电机驱动 | 右电机 PWM 调速 |
| LED_FRONT | GPIOxx | 前灯板 | 前灯控制 |
| LED_BACK | GPIOxx | 后灯板 | 后灯控制 |
| BAT_ADC | ADC | 磁吸板 5V | 电池电压检测 |

### 马达驱动电路 | Motor Driver

底盘使用双路 H 桥驱动两颗 N20 减速电机：

```
         ┌─────────────────────┐
GPIOx ───│ IN1   H桥   OUT1 ───│── 左电机 +
GPIOx ───│ IN2         OUT2 ───│── 左电机 -
GPIOx ───│ PWM (EN)            │
         │                     │
GPIOx ───│ IN3   H桥   OUT3 ───│── 右电机 +
GPIOx ───│ IN4         OUT4 ───│── 右电机 -
GPIOx ───│ PWM (EN)            │
         └─────────────────────┘
```

> 电机驱动 IC 的具体型号和 H 桥连线方式以开源原理图为准。

### 飞线连接总览 | Wire Routing Overview

```
    ┌──────────────┐
    │  主控板       │
    │ (ESP8684)    │
    └──┬───┬───┬───┘
       │   │   │
       │   │   └──────── 飞线 ────────┐
       │   │                          │
       │   └────────── 飞线 ──────┐   │
       │                          │   │
  ┌────┴────┐  ┌────────┐  ┌──────┴───┴──┐
  │ 前灯板   │  │ 后灯板  │  │  磁吸板      │
  └─────────┘  └────────┘  └─────────────┘
                                  │
                        4P pogopin 公头
                        (吸附底板底部 4P 母座)
```

---

## 上电前检查 | Pre-Power Checklist

在每次上电之前，务必完成以下检查：

```
□ 万用表蜂鸣档：VCC-GND 不短路（所有 PCB 单独测试）
□ 万用表蜂鸣档：每对相邻 GPIO 引脚之间不短路
□ 排针排母方向正确（对照 PCB 丝印方向标记）
□ 电池插头正负极正确（红+ 黑-）
□ USB-C 焊接无连锡
□ 所有 FPC 排线插紧、方向正确
□ 底盘飞线无虚焊、无短接
□ 3.3V 测试点对地阻抗正常 (通常 >1kΩ)
```

---

*最后更新：2026-05-27*
