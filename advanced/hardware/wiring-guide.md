# AICat 接线指南 / AICat Wiring Guide

> Advanced Track - Desktop Pet Hardware
> 高级赛道 - 桌面宠物硬件

---

## 整体接线概览 / Overall Wiring Overview

```
                          ┌─────────────────────────┐
                          │    XIAO ESP32S3 Sense    │
                          │                         │
                          │  USB-C (供电 + 下载)     │
                          │                         │
              D0/GPIO41 ──┤ CS    SDA/D4/GPIO5 ────┤──→ PCA9685 SDA
              D1/GPIO42 ──┤ DC    SCL/D5/GPIO6 ────┤──→ PCA9685 SCL
              EN/RST ─────┤ RST   TX/D6/GPIO43 ────┤──→ STS3032 TX (黄线)
              D8/GPIO9  ──┤ SCK   RX/D7/GPIO44 ────┤──→ STS3032 RX (黄线)
              D10/GPIO7 ──┤ MOSI                    │
                          │                         │
              D3/GPIO4  ──┤ BCLK ──→ MAX98357A BCLK │
              D2/GPIO3  ──┤ LRC  ──→ MAX98357A LRC  │
              D9/GPIO8  ──┤ DIN  ──→ MAX98357A DIN  │
                          │                         │
                          │  3.3V ──→ ST7789 VCC    │
                          │         PCA9685 VCC     │
                          │                         │
                          │  5V   ──→ MAX98357A VIN │
                          │                         │
                          │  GND  ──→ 公共地         │
                          └─────────────────────────┘
                                     │
    ┌────────────────────────────────┼──────────────────────────────┐
    │                                │                              │
    ▼                                ▼                              ▼
 ┌─────────┐                  ┌───────────┐                  ┌──────────┐
 │  ST7789  │                  │  PCA9685  │                  │ MAX98357A│
 │  LCD屏   │                  │  PWM驱动  │                  │  功放模块 │
 │ (SPI总线)│                  │ (I2C总线) │                  │ (I2S总线) │
 └─────────┘                  └─────┬─────┘                  └────┬─────┘
                                     │                             │
                          ┌──────────┼──────────┐                  │
                          │          │          │                  ▼
                          ▼          ▼          ▼            ┌──────────┐
                       ┌──────┐ ┌──────┐ ┌──────┐           │  小喇叭  │
                       │MG90S │ │MG90S │ │微型  │           │  4Ω 3W  │
                       │左耳  │ │右耳  │ │x3   │           └──────────┘
                       │CH14  │ │CH15  │ │CH12,│
                       └──────┘ └──────┘ │CH13 │
                                          └──────┘

    ┌─────────────────────────────────────────────────────────────────────┐
    │                     外部电源 5V/6V                                  │
    │           ┌──────────────────────────────────────────┐              │
    │           │  V+ ──→ PCA9685 V+ (舵机动力电源)         │              │
    │           │  GND ──→ 公共地 (与 XIAO GND 相连)        │              │
    │           │  V+ ──→ STS3032 VCC (总线舵机电源)        │              │
    │           └──────────────────────────────────────────┘              │
    └─────────────────────────────────────────────────────────────────────┘
```

---

## 重要规则 / Important Rules

### 1. 公共地 / Common Ground

```
    所有模块的 GND 必须连接在一起!
    All module GND pins MUST be connected together!

    XIAO GND ──┬── ST7789 GND
               ├── PCA9685 GND
               ├── MAX98357A GND
               ├── 外部电源 GND
               └── STS3032 GND

    ⚠️ 如果没有公共地, 信号将无法正常工作!
    ⚠️ Without common ground, signals will NOT work properly!
```

### 2. 舵机独立供电 / Separate Servo Power

```
    ⛔ 错误 / WRONG:
    XIAO 5V ──→ 舵机电源    (电流不够, 会导致 ESP32 重启!)

    ✅ 正确 / CORRECT:
    外部 5V/6V 电源 ──→ 舵机电源 (PCA9685 V+, STS3032 VCC)
    外部电源 GND   ──→ XIAO GND  (共地)
```

### 3. 逻辑电平 / Logic Levels

```
    XIAO ESP32S3 GPIO: 3.3V 逻辑电平
    PCA9685: 支持 3.3V 逻辑 (VCC 接 3.3V)
    ST7789: 3.3V 逻辑
    MAX98357A: 支持 3.3V 逻辑输入
    STS3032: TTL 电平, 兼容 3.3V

    所有模块都可以直接与 XIAO 连接, 无需电平转换!
    All modules can connect directly to XIAO — no level shifter needed!
```

---

## 1. ST7789 LCD 显示屏 / ST7789 LCD Display

### 接线表 / Wiring Table

| ST7789 引脚 / Pin | XIAO 引脚 / Pin | GPIO | 说明 / Description |
|---|---|---|---|
| VCC | 3.3V | - | 电源 (勿接 5V!) / Power (do NOT use 5V!) |
| GND | GND | - | 地 / Ground |
| CS | D0 | GPIO41 | 片选 / Chip Select |
| DC (RS) | D1 | GPIO42 | 数据/命令 / Data/Command |
| RST | EN (RST) | - | 复位 (可直接接 XIAO RST) / Reset |
| SCK (CLK) | D8 | GPIO9 | SPI 时钟 / SPI Clock |
| MOSI (SDA) | D10 | GPIO7 | SPI 数据 / SPI Data |
| BL (背光) | 3.3V | - | 背光电源 / Backlight power |

### 接线图 / Connection Diagram

```
    XIAO ESP32S3 Sense                ST7789 LCD
    ┌──────────────┐                 ┌──────────┐
    │         3.3V ├─────────────────┤ VCC      │
    │          GND ├──────────┬──────┤ GND      │
    │              │          │      │          │
    │     D0/GPIO41├──────────┼──────┤ CS       │
    │     D1/GPIO42├──────────┼──────┤ DC       │
    │     EN (RST) ├──────────┼──────┤ RST      │
    │      D8/GPIO9├──────────┼──────┤ SCK      │
    │      D10/GPIO7├─────────┼──────┤ MOSI     │
    │         3.3V ├──────────┼──────┤ BL       │
    └──────────────┘          │      └──────────┘
                              │
                           公共地
```

> **注意 / Note**: ST7789 的 MOSI 引脚有时标注为 SDA (这是 SPI 的数据线, 不是 I2C 的 SDA).
> The ST7789's MOSI pin is sometimes labeled SDA (this is SPI data, NOT I2C SDA).

---

## 2. PCA9685 PWM 舵机驱动 / PCA9685 PWM Servo Driver

### 接线表 / Wiring Table

| PCA9685 引脚 / Pin | XIAO / 电源 | 说明 / Description |
|---|---|---|
| SDA | D4 / GPIO5 | I2C 数据 / I2C Data |
| SCL | D5 / GPIO6 | I2C 时钟 / I2C Clock |
| VCC | 3.3V | 逻辑电源 (仅驱动芯片) / Logic power (chip only) |
| V+ | 外部 5V/6V | 舵机动力电源 / Servo motor power |
| GND | GND (公共地) | 地 / Ground |
| OE | 悬空或 GND | 输出使能 (低有效) / Output Enable (active low) |

### 舵机通道分配 / Servo Channel Assignment

| 通道 / Channel | 舵机 / Servo | 类型 / Type | 说明 / Description |
|---|---|---|---|
| CH0-CH11 | 未使用 / Unused | - | 预留 / Reserved |
| CH12 | 嘴巴 / Mouth | 微型舵机 2g/3.7g | 张嘴/闭嘴 / Open/close mouth |
| CH13 | 尾巴 / Tail | 微型舵机 2g/3.7g | 摇尾巴 / Wag tail |
| CH14 | 左耳 / Left Ear | MG90S | 耳朵动作 / Ear movement |
| CH15 | 右耳 / Right Ear | MG90S | 耳朵动作 / Ear movement |

### 接线图 / Connection Diagram

```
    XIAO ESP32S3 Sense              PCA9685
    ┌──────────────┐               ┌──────────────────────────────┐
    │         3.3V ├───────────────┤ VCC (逻辑电源)               │
    │          GND ├──────────┬────┤ GND                          │
    │              │          │    │                              │
    │     D4/GPIO5 ├──────────┼────┤ SDA                          │
    │     D5/GPIO6 ├──────────┼────┤ SCL                          │
    └──────────────┘          │    │                              │                              │
                              │    │  CH12 ──→ 微型舵机 (嘴巴)    │
    ┌──────────────┐          │    │  CH13 ──→ 微型舵机 (尾巴)    │
    │ 外部电源 5V/6V│          │    │  CH14 ──→ MG90S  (左耳)     │
    │          V+  ├──────────┼────┤ V+  (舵机动力电源)           │
    │          GND ├──────────┘    │                              │
    └──────────────┘               └──────────────────────────────┘

    PCA9685 舵机连接细节:
    每个舵机有 3 根线: 信号(黄/橙), V+(红), GND(棕/黑)
    直接插入 PCA9685 对应通道即可 (注意方向!)

    Servo connection detail:
    Each servo has 3 wires: Signal(yellow/orange), V+(red), GND(brown/black)
    Plug directly into PCA9685 channel (mind the polarity!)
```

> **注意 / Note**: PCA9685 的 I2C 地址默认为 0x40. 如果需要级联多个 PCA9685,
> 可以通过板上的 A0-A5 焊盘修改地址 (0x40 - 0x7F).
> Default I2C address is 0x40. For cascading, modify A0-A5 solder pads.

---

## 3. STS3032 总线舵机 / STS3032 Bus Servos

### 接线表 / Wiring Table

| STS3032 总线 / Bus | XIAO 引脚 / Pin | GPIO | 说明 / Description |
|---|---|---|---|
| TX (发送) | D6 | GPIO43 | XIAO TX → STS3032 数据线 |
| RX (接收) | D7 | GPIO44 | XIAO RX ← STS3032 数据线 |
| VCC | 外部 5V/6V | - | 舵机电源 / Servo power |
| GND | GND (公共地) | - | 地 / Ground |

### 舵机 ID 分配 / Servo ID Assignment

| 舵机 ID | 位置 / Position | 说明 / Description |
|---|---|---|
| ID 1 | 左前腿 / Left Front Leg | 左前腿关节 / Left front leg joint |
| ID 2 | 右前腿 / Right Front Leg | 右前腿关节 / Right front leg joint |
| ID 3 | 左后腿 / Left Rear Leg | 左后腿关节 / Left rear leg joint |
| ID 4 | 右后腿 / Right Rear Leg | 右后腿关节 / Right rear leg joint |

### 接线图 / Connection Diagram

```
    XIAO ESP32S3 Sense              STS3032 总线舵机
    ┌──────────────┐               ┌──────────────────────┐
    │              │               │                      │
    │   D6/GPIO43  ├───────────────┤ 数据线 (TX/RX 共用)  │
    │   D7/GPIO44  ├───────────────┤ (半双工 TTL 总线)     │
    │              │               │                      │
    │          GND ├──────────┬────┤ GND                  │
    └──────────────┘          │    └──────────────────────┘
                              │
    ┌──────────────┐          │
    │ 外部电源 5V/6V│          │
    │          V+  ├──────────┼──── STS3032 VCC (6V 推荐)
    │          GND ├──────────┘
    └──────────────┘

    总线舵机串联连接:
    Bus servos are daisy-chained:

    XIAO ──→ [ID1 左前腿] ──→ [ID2 右前腿] ──→ [ID3 左后腿] ──→ [ID4 右后腿]

    每个舵机有 3 孔接口: VCC, GND, DATA
    串联时数据线依次连接, 电源线并联供电
```

### STS3032 特殊注意事项 / STS3032 Special Notes

1. **半双工通信 / Half-Duplex**: STS3032 使用单线 TTL 通信, TX 和 RX 共用一根数据线. 需要使用 TTL 适配器或软件实现半双工.
2. **波特率 / Baud Rate**: 默认 1000000 bps (1Mbps). 首次使用可能需要通过适配器确认.
3. **舵机 ID / Servo ID**: 出厂默认 ID 通常为 1. 使用 TTL 适配器 + 上位机软件 逐一修改 ID (1-4).
4. **电压 / Voltage**: STS3032 推荐 6V 供电, 可在 5V-7.4V 范围内工作.

> **首次使用前 / Before First Use**: 必须用 TTL 适配器连接电脑, 使用上位机软件确认每个舵机的 ID 并逐一设置为 1-4.
> You MUST use the TTL adapter to connect to a PC and configure each servo's ID to 1-4 before wiring to XIAO.

---

## 4. MAX98357A I2S 功放 / MAX98357A I2S Amplifier

### 接线表 / Wiring Table

| MAX98357A 引脚 / Pin | XIAO 引脚 / Pin | GPIO | 说明 / Description |
|---|---|---|---|
| BCLK | D3 | GPIO4 | I2S 位时钟 / I2S Bit Clock |
| LRC | D2 | GPIO3 | I2S 左右声道选择 / I2S Left-Right Clock |
| DIN | D9 | GPIO8 | I2S 数据输入 / I2S Data In |
| VIN | 5V | - | 电源 (接 5V) / Power (5V) |
| GND | GND (公共地) | - | 地 / Ground |
| GAIN | 悬空或接电阻 | - | 增益设定 (默认 9dB) / Gain setting |
| SD_MODE | 悬空或接高 | - | 关断模式 (默认播放) / Shutdown mode |

### 接线图 / Connection Diagram

```
    XIAO ESP32S3 Sense              MAX98357A              小喇叭
    ┌──────────────┐               ┌──────────┐          ┌──────────┐
    │         5V   ├───────────────┤ VIN      │          │          │
    │         GND  ├──────────┬────┤ GND      │   ┌──────┤  + (红)  │
    │              │          │    │          │   │      │  - (黑)  │
    │    D3/GPIO4  ├──────────┼────┤ BCLK     │   │      └──────────┘
    │    D2/GPIO3  ├──────────┼────┤ LRC      ├───┘
    │    D9/GPIO8  ├──────────┼────┤ DIN      │
    └──────────────┘          │    │          │
                              │    │  喇叭接Speaker+/-│
                              │    └──────────┘
                           公共地

    MAX98357A 增益设置 (GAIN 引脚):
    ┌─────────────┬────────────────┐
    │ GAIN 接法    │ 增益            │
    ├─────────────┼────────────────┤
    │ 悬空         │ 9 dB (默认)     │
    │ 接 GND       │ 6 dB           │
    │ 接 VCC       │ 12 dB          │
    │ 接 100k到GND │ 15 dB          │
    └─────────────┴────────────────┘
```

> **注意 / Note**: 喇叭的正负极不能短路! 喇叭线不要碰到面包板或其他金属.
> Speaker wires must not short-circuit! Keep them away from metal contacts on the breadboard.

---

## 5. 摄像头与麦克风 / Camera & Microphone

```
    XIAO ESP32S3 Sense 自带摄像头和麦克风, 无需额外接线!

    ┌──────────────────────────┐
    │   XIAO ESP32S3 Sense     │
    │                          │
    │   ┌──────────┐           │
    │   │ OV2640   │ ← 摄像头  │
    │   │ 摄像头模块│   (已焊接) │
    │   └──────────┘           │
    │                          │
    │   ┌──────────┐           │
    │   │ PDM 数字  │ ← 麦克风  │
    │   │ 麦克风    │   (已焊接) │
    │   └──────────┘           │
    │                          │
    └──────────────────────────┘

    ✓ 无需额外接线 / No extra wiring needed
    ✓ 摄像头通过内部 DVP 接口连接 / Camera via internal DVP bus
    ✓ 麦克风通过内部 I2S/PDM 接口连接 / Mic via internal I2S/PDM bus
```

> **确认 Sense 版本 / Verify Sense Version**: 如果你的板子上没有看到摄像头模块, 说明你买到的是普通版而不是 Sense 版, 请重新购买.
> If you don't see a camera module on your board, you have the standard version, not Sense. Please repurchase.

---

## 完整接线汇总表 / Complete Pin Summary

| XIAO Pin | GPIO | 功能 / Function | 连接到 / Connected To |
|---|---|---|---|
| D0 | GPIO41 | SPI CS | ST7789 CS |
| D1 | GPIO42 | SPI DC | ST7789 DC |
| D2 | GPIO3 | I2S LRC | MAX98357A LRC |
| D3 | GPIO4 | I2S BCLK | MAX98357A BCLK |
| D4 | GPIO5 | I2C SDA | PCA9685 SDA |
| D5 | GPIO6 | I2C SCL | PCA9685 SCL |
| D6 | GPIO43 | UART TX | STS3032 数据线 |
| D7 | GPIO44 | UART RX | STS3032 数据线 |
| D8 | GPIO9 | SPI SCK | ST7789 SCK |
| D9 | GPIO8 | I2S DIN | MAX98357A DIN |
| D10 | GPIO7 | SPI MOSI | ST7789 MOSI |
| EN/RST | - | Reset | ST7789 RST |
| 3.3V | - | 3.3V 电源 | ST7789 VCC, PCA9685 VCC |
| 5V | - | 5V 电源 | MAX98357A VIN |
| GND | - | 地 | 公共地 (所有模块) |

---

## 面包板布局建议 / Breadboard Layout Suggestion

```
    ┌──────────────────────────────────────────────────────────────────┐
    │                         面包板 / Breadboard                      │
    │                                                                  │
    │  ┌─────────┐                                                     │
    │  │  XIAO   │   ┌───────┐   ┌───────────┐                        │
    │  │ ESP32S3 │   │ST7789 │   │  PCA9685  │                        │
    │  │  Sense  │   │ LCD   │   │  PWM驱动   │                        │
    │  └─────────┘   └───────┘   └───────────┘                        │
    │                                                                  │
    │  ┌───────────┐  ┌──────────────────────┐                        │
    │  │ MAX98357A │  │    外部电源接入点      │                        │
    │  │  I2S 功放  │  │  (5V/6V GND 共地)     │                        │
    │  └─────┬─────┘  └──────────────────────┘                        │
    │        │                                                         │
    │      喇叭                                                        │
    │                                                                  │
    │  STS3032 总线舵机通过杜邦线直接连接到 D6/D7                       │
    │  (总线舵机安装在猫咪外壳内, 通过延长线连接面包板)                  │
    └──────────────────────────────────────────────────────────────────┘

    建议:
    - XIAO 放在面包板中央, 方便向两侧走线
    - 电源线用红色, 地线用黑色/蓝色, 信号线用其他颜色
    - 舵机电源线 (红/黑) 尽量短且粗, 减少压降

    Tips:
    - Place XIAO in the center for easy routing to both sides
    - Use red for power, black/blue for ground, other colors for signals
    - Keep servo power wires short and thick to reduce voltage drop
```

---

## 首次通电检查清单 / First Power-Up Checklist

在给系统通电之前, 请逐项检查:

Before powering up, check each item:

- [ ] 所有 GND 已连接在一起 / All GND pins connected together
- [ ] ST7789 VCC 接 3.3V (不是 5V) / ST7789 VCC on 3.3V (not 5V)
- [ ] 舵机电源 (PCA9685 V+, STS3032 VCC) 来自外部电源 / Servo power from external supply
- [ ] 外部电源 GND 与 XIAO GND 相连 / External supply GND connected to XIAO GND
- [ ] 没有短路 (用万用表检查 VCC 和 GND 之间的电阻) / No short circuits (check resistance between VCC and GND)
- [ ] USB 数据线已连接 XIAO / USB data cable connected to XIAO
- [ ] 喇叭已连接到 MAX98357A / Speaker connected to MAX98357A
- [ ] 所有杜邦线已插紧 / All Dupont wires firmly seated
