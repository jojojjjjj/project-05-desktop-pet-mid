# AICat 物料清单 / AICat Bill of Materials

> Advanced Track - Desktop Pet Hardware
> 高级赛道 - 桌面宠物硬件

---

## 系统架构 / System Architecture

```
                    ┌──────────────────────────────┐
                    │      XIAO ESP32S3 Sense       │
                    │   ┌────────┐   ┌──────────┐   │
                    │   │ Camera │   │   Mic    │   │
                    │   │ OV2640 │   │  PDM数字 │   │
                    │   └────────┘   └──────────┘   │
                    │                              │
                    │   GPIO   I2C    SPI    UART   │
                    └──┬───────┬───────┬───────┬───┘
                       │       │       │       │
          ┌────────────┘       │       │       │
          │                    │       │       │
          ▼                    ▼       ▼       ▼
    ┌───────────┐      ┌──────────┐ ┌─────┐ ┌────────────┐
    │ MAX98357A │      │ PCA9685  │ │ST7789│ │ STS3032    │
    │ I2S 功放  │      │PWM 驱动  │ │LCD  │ │ 总线舵机   │
    │  (I2S)    │      │  (I2C)   │ │(SPI)│ │  (UART)    │
    └─────┬─────┘      └────┬─────┘ └─────┘ └─────┬──────┘
          │                 │                     │
          ▼                 ▼                     ▼
    ┌───────────┐   ┌───────────────┐     ┌────────────┐
    │  小喇叭   │   │ 舵机组 x7     │     │ 总线舵机   │
    │  4Ω 3W   │   │ 嘴/尾/左耳/右耳│     │  x4 (腿部) │
    └───────────┘   └───────────────┘     └────────────┘

    ┌─────────────────────────────────────────────────────┐
    │            外部电源 5V/6V (舵机独立供电)            │
    │         External Power 5V/6V (Servo Power)          │
    └─────────────────────────────────────────────────────┘
```

---

## 核心控制器 / Core Controller

| # | 组件 / Component | 规格 / Specs | 数量 / Qty | 单价 / Unit Price | 小计 / Subtotal |
|---|---|---|---|---|---|
| 1 | XIAO ESP32S3 **Sense** | WiFi+BLE, OV2640 摄像头, PDM 数字麦克风, 240MHz 双核, 8MB PSRAM, 8MB Flash | 1 | ~60 元 | ~60 元 |

> **重要**: 必须购买 **Sense** 版本! 普通版没有摄像头和麦克风。
> **IMPORTANT**: You MUST buy the **Sense** version! The standard version has no camera or microphone.

**淘宝搜索关键词 / Taobao Search Keywords:**
- `XIAO ESP32S3 Sense`
- `Seeed Studio XIAO ESP32S3 Sense 开发板`
- `XIAO ESP32S3 摄像头 麦克风`

---

## 显示屏 / Display

| # | 组件 / Component | 规格 / Specs | 数量 / Qty | 单价 / Unit Price | 小计 / Subtotal |
|---|---|---|---|---|---|
| 2 | ST7789 SPI LCD | 1.83 英寸, 170x320 像素, SPI 接口, 3.3V/5V 兼容 | 1 | ~15 元 | ~15 元 |

**淘宝搜索关键词 / Taobao Search Keywords:**
- `ST7789 1.83寸 170x320 SPI LCD`
- `1.83寸 TFT LCD ST7789 液晶屏模块`
- `ST7789 170x320 彩色屏幕模块`

---

## 音频 / Audio

| # | 组件 / Component | 规格 / Specs | 数量 / Qty | 单价 / Unit Price | 小计 / Subtotal |
|---|---|---|---|---|---|
| 3 | MAX98357A I2S 功放模块 | I2S 输入, 3W D类功放, 无需外部滤波 | 1 | ~8 元 | ~8 元 |
| 4 | 小型喇叭 | 4Ω / 3W, 28mm-40mm 直径 | 1 | ~5 元 | ~5 元 |

**淘宝搜索关键词 / Taobao Search Keywords:**
- `MAX98357A I2S 功放模块`
- `MAX98357 I2S amplifier module`
- `小喇叭 4Ω 3W 28mm` / `小喇叭 4欧 3瓦`

---

## 舵机驱动 / Servo Driver

| # | 组件 / Component | 规格 / Specs | 数量 / Qty | 单价 / Unit Price | 小计 / Subtotal |
|---|---|---|---|---|---|
| 5 | PCA9685 16 路 PWM 驱动 | I2C 接口, 16 通道, 12-bit PWM, 可级联 | 1 | ~12 元 | ~12 元 |

**淘宝搜索关键词 / Taobao Search Keywords:**
- `PCA9685 16路 PWM 舵机驱动`
- `PCA9685 servo driver module`
- `Adafruit PCA9685 16通道`

---

## 舵机 / Servos

### 总线舵机 (腿部) / Bus Servos (Legs)

| # | 组件 / Component | 规格 / Specs | 数量 / Qty | 单价 / Unit Price | 小计 / Subtotal |
|---|---|---|---|---|---|
| 6 | STS3032 总线舵机 | TTL 串行通信, 360° 可控, 带反馈, 扭力 ~4.6kg.cm | 4 | ~40 元 | ~160 元 |

**淘宝搜索关键词 / Taobao Search Keywords:**
- `STS3032 总线舵机`
- `飞特 STS3032 TTL 总线舵机`
- `FEETECH STS3032 serial servo`

### 微型舵机 (嘴/尾) / Micro Servos (Mouth/Tail)

| # | 组件 / Component | 规格 / Specs | 数量 / Qty | 单价 / Unit Price | 小计 / Subtotal |
|---|---|---|---|---|---|
| 7 | 2g / 3.7g 微型舵机 | 模拟 PWM, 3.7g 重量, 180°, 用于嘴和尾巴 | 3 | ~8 元 | ~24 元 |

**淘宝搜索关键词 / Taobao Search Keywords:**
- `2g 微型舵机` / `3.7g 舵机`
- `微型舵机 2克 3.7克 180度`
- `micro servo 2g 3.7g`

### MG90S 舵机 (耳朵) / MG90S Servos (Ears)

| # | 组件 / Component | 规格 / Specs | 数量 / Qty | 单价 / Unit Price | 小计 / Subtotal |
|---|---|---|---|---|---|
| 8 | MG90S 金属齿轮舵机 | 金属齿轮, 180°, 扭力 ~1.8kg.cm, 用于耳朵 | 2 | ~12 元 | ~24 元 |

**淘宝搜索关键词 / Taobao Search Keywords:**
- `MG90S 金属齿轮舵机`
- `MG90S servo 金属舵机`
- `MG90S 9g 舵机 180度`

---

## 总线舵机适配器 / Bus Servo Adapter

| # | 组件 / Component | 规格 / Specs | 数量 / Qty | 单价 / Unit Price | 小计 / Subtotal |
|---|---|---|---|---|---|
| 9 | TTL 总线舵机转接板 | USB/UART 转 TTL, 用于设置舵机 ID 和测试 | 1 | ~15 元 | ~15 元 |

**淘宝搜索关键词 / Taobao Search Keywords:**
- `总线舵机 TTL 转接板`
- `FEETECH TTL bus servo adapter`
- `飞特总线舵机调试器 USB转TTL`

---

## 电源 / Power Supply

| # | 组件 / Component | 规格 / Specs | 数量 / Qty | 单价 / Unit Price | 小计 / Subtotal |
|---|---|---|---|---|---|
| 10 | 外部电源 5V/6V | 5V 或 6V, >= 3A 输出, 舵机独立供电 | 1 | ~20 元 | ~20 元 |

> **注意**: 舵机不能从 XIAO 的 3.3V/5V 供电! 必须使用独立电源。
> **NOTE**: Do NOT power servos from XIAO's 3.3V/5V pin! Use a separate power supply.

**淘宝搜索关键词 / Taobao Search Keywords:**
- `5V 3A 开关电源模块`
- `5V 6V 大电流电源适配器`
- `降压模块 5V 3A 舵机供电`

---

## 辅材 / Accessories

| # | 组件 / Component | 规格 / Specs | 数量 / Qty | 单价 / Unit Price | 小计 / Subtotal |
|---|---|---|---|---|---|
| 11 | 面包板 | 400 孔标准面包板 | 1 | ~5 元 | ~5 元 |
| 12 | 杜邦线 | 公对公 + 公对母, 各 20 根 | 1 套 | ~5 元 | ~5 元 |
| 13 | USB 数据线 | Type-C, 支持数据传输 (不仅是充电线) | 1 | ~5 元 | ~5 元 |
| 14 | 3D 打印猫咪外壳 | PLA 材料, FDM 打印 (STL 文件见项目仓库) | 1 套 | ~20 元 | ~20 元 |

---

## 费用汇总 / Cost Summary

| 类别 / Category | 金额 / Cost |
|---|---|
| 核心控制器 XIAO ESP32S3 Sense | ~60 元 |
| 显示屏 ST7789 LCD | ~15 元 |
| 音频 (功放 + 喇叭) | ~13 元 |
| 舵机驱动 PCA9685 | ~12 元 |
| 舵机 STS3032 x4 (腿部) | ~160 元 |
| 舵机 微型 x3 (嘴/尾) | ~24 元 |
| 舵机 MG90S x2 (耳朵) | ~24 元 |
| 总线舵机适配器 | ~15 元 |
| 外部电源 | ~20 元 |
| 辅材 (面包板/线材/USB/3D打印) | ~35 元 |
| **总计 / Total** | **~378 元** |

> 预估总价范围: **350 ~ 400 元** (因商家和运费而异)
> Estimated total range: **350 ~ 400 RMB** (varies by seller and shipping)

---

## 购买优先级 / Purchase Priority

如果需要分批购买,建议按以下顺序:

If you need to buy in batches, follow this order:

1. **第一批 / Batch 1** (先验证核心功能): XIAO ESP32S3 Sense + ST7789 LCD + MAX98357A + 小喇叭
2. **第二批 / Batch 2** (舵机系统): PCA9685 + 微型舵机 + MG90S + 外部电源 + 面包板/线材
3. **第三批 / Batch 3** (腿部运动): STS3032 x4 + TTL 适配器
4. **第四批 / Batch 4** (外壳): 3D 打印猫咪外壳

---

## 替代方案 / Alternatives

| 原组件 / Original | 替代方案 / Alternative | 备注 / Notes |
|---|---|---|
| ST7789 1.83" | ST7789 1.54" 240x240 | 更常见,但分辨率不同,需修改代码 |
| STS3032 总线舵机 | SC0002 / ST3215 总线舵机 | 兼容 TTL 协议,需确认 ID 设置方式 |
| MAX98357A | NS4150 / PAM8302 | 不同功放芯片,接线可能不同 |
| MG90S 耳机舵机 | SG90 塑料齿轮舵机 | 更便宜 (~5 元) 但扭力小,齿轮不耐久 |
