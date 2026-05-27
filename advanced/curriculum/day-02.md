# Day 02: 硬件认知与舵机控制基础 | Hardware Overview & Servo Control Basics

> **今日目标:**
> - 认识桌面宠物项目的所有硬件组件及其功能
> - 理解I2C通信协议与PCA9685 PWM驱动器的工作原理
> - 掌握舵机控制理论（PWM信号、占空比）
> - 完成第一个舵机扫描程序
> - 了解STS3032总线舵机的基本概念
>
> **产出:** 能够通过PCA9685控制舵机旋转，完成舵机扫描程序

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间回顾 | 回顾昨日作业，解答疑问 |
| 09:15 - 10:30 | 知识讲解 | 硬件组件总览与I2C协议 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | PCA9685接线与舵机控制 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | 舵机扫描程序与多舵机控制 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | STS3032总线舵机入门 |
| 16:30 - 17:00 | 总结分享 | 今日收获总结 |

## 上午: 硬件组件与I2C协议 | Morning: Hardware Components & I2C Protocol

### 1. 硬件组件总览 | Hardware Component Overview

本项目使用的所有硬件组件：

All hardware components used in this project:

| 组件 Component | 数量 | 功能 Function |
|---------------|------|--------------|
| XIAO ESP32S3 Sense | 1 | 主控制器，含摄像头和麦克风 Main MCU with camera & mic |
| ST7789 1.47" TFT屏幕 | 1 | 显示表情动画 Display for expressions |
| PCA9685 PWM驱动板 | 1 | 16路PWM输出，驱动舵机 16-ch PWM for servos |
| SG90舵机 (9g) | 4 | 控制耳朵、尾巴、嘴巴 Micro servos for ears/tail/mouth |
| STS3032总线舵机 | 4 | 控制腿部行走 Bus servos for leg movement |
| MAX98357A功放模块 | 1 | I2S音频输出，驱动喇叭 I2S audio amplifier |
| 小喇叭 8Ω 1W | 1 | 发声 Speaker |
| 面包板 | 1 | 原型搭建 Prototyping |
| 杜邦线 | 若干 | 连接 Wiring |

### 2. I2C通信协议 | I2C Communication Protocol

I2C（Inter-Integrated Circuit）是一种两线制串行通信协议，非常适合连接低速外设：

I2C (Inter-Integrated Circuit) is a two-wire serial communication protocol ideal for connecting low-speed peripherals:

```
      主机 Master (XIAO ESP32S3)
        │
        ├── SDA (数据线 Data) ←→ PCA9685 SDA
        │                    ←→ 其他I2C设备
        │
        └── SCL (时钟线 Clock) ←→ PCA9685 SCL
                              ←→ 其他I2C设备
```

XIAO ESP32S3 I2C引脚：
- **SDA:** D6 (GPIO6)
- **SCL:** D7 (GPIO7)

I2C关键概念 | Key I2C Concepts:
- 每个设备有唯一的7位地址（PCA9685默认地址: `0x40`）
- 通信速率：标准模式100kHz，快速模式400kHz
- 主从模式：主机发起通信，从机响应
- 需要上拉电阻（PCA9685模块通常已集成）

### 3. PCA9685 PWM驱动器 | PCA9685 PWM Driver

PCA9685是一个16路12位PWM/Servo驱动器，通过I2C控制：

The PCA9685 is a 16-channel 12-bit PWM/Servo driver controlled via I2C:

```
PCA9685 引脚连接:
  VCC  → 5V (逻辑电源 Logic power)
  GND  → GND
  SDA  → XIAO D6
  SCL  → XIAO D7
  V+   → 5V (舵机电源 Servo power, 需要大电流 separate supply recommended)
```

PCA9685在本项目中的通道分配：
| 通道 Channel | 功能 Function |
|-------------|--------------|
| CH12 | 嘴巴 Mouth |
| CH13 | 尾巴 Tail |
| CH14 | 左耳 Left Ear |
| CH15 | 右耳 Right Ear |

### 4. 舵机控制理论 | Servo Control Theory

标准舵机通过PWM信号控制角度：

Standard servos are controlled via PWM signals to set the angle:

```
PWM频率: 50Hz (周期20ms)
脉冲宽度范围: 0.5ms ~ 2.5ms

0.5ms  →   0° (最小角度)
1.5ms  →  90° (中间位置)
2.5ms  → 180° (最大角度)

┌─┐               ┌─┐               ┌─┐
│◀0.5ms▶│         │◀1.5ms▶│         │◀2.5ms▶│
│ │               │               │
│ │◀─── 20ms ───▶│               │
  0°               90°             180°
```

12位分辨率下的占空比计算 | Duty cycle with 12-bit resolution:

```
PCA9685 12位分辨率: 0 ~ 4095
PWM频率: 50Hz → 周期 = 20ms = 20000μs
每个计数 = 20000μs / 4096 ≈ 4.88μs

脉冲宽度 → 计数值:
  0.5ms = 500μs  → 500 / 4.88  ≈ 102
  1.5ms = 1500μs → 1500 / 4.88 ≈ 307
  2.5ms = 2500μs → 2500 / 4.88 ≈ 512
```

## 下午: 舵机控制实践 | Afternoon: Servo Control Practice

### 5. PCA9685初始化 | PCA9685 Initialization

```cpp
// Day 02 - PCA9685 基础初始化
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// 创建PCA9685对象，默认地址0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// 舵机脉冲范围定义
#define SERVOMIN  102   // 0°   对应脉冲计数 (0.5ms)
#define SERVOMAX  512   // 180° 对应脉冲计数 (2.5ms)
#define SERVO_FREQ 50   // 舵机PWM频率 50Hz

// 通道定义
#define MOUTH_PIN  12
#define TAIL_PIN   13
#define LEFT_EAR   14
#define RIGHT_EAR  15

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== PCA9685 舵机控制测试 ===");

  // 初始化I2C
  Wire.begin(6, 7);  // SDA=D6, SCL=D7

  // 初始化PCA9685
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);  // 内部振荡器频率
  pwm.setPWMFreq(SERVO_FREQ);           // 设置PWM频率为50Hz

  Serial.println("PCA9685 初始化完成!");
  delay(100);  // 等待PCA9685稳定
}

// 将角度(0-180)转换为PWM脉冲值
int angleToPulse(int angle) {
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;
  return map(angle, 0, 180, SERVOMIN, SERVOMAX);
}

void loop() {
  // 空循环，等待串口命令
}
```

### 6. 舵机扫描程序 | Servo Sweep Program

```cpp
// Day 02 - 舵机扫描 (Sweep) 程序
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

#define SERVOMIN  102
#define SERVOMAX  512
#define SERVO_FREQ 50

int angleToPulse(int angle) {
  return map(constrain(angle, 0, 180), 0, 180, SERVOMIN, SERVOMAX);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(6, 7);
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);
  delay(100);
  Serial.println("=== 舵机扫描测试 ===");
}

void sweepServo(uint8_t channel, const char* name) {
  Serial.printf("扫描 %s (CH%d):\n", name, channel);

  // 从0°到180°
  for (int angle = 0; angle <= 180; angle += 5) {
    pwm.setPWM(channel, 0, angleToPulse(angle));
    Serial.printf("  角度: %3d°\n", angle);
    delay(30);
  }

  delay(500);

  // 从180°到0°
  for (int angle = 180; angle >= 0; angle -= 5) {
    pwm.setPWM(channel, 0, angleToPulse(angle));
    Serial.printf("  角度: %3d°\n", angle);
    delay(30);
  }

  // 回到中位
  pwm.setPWM(channel, 0, angleToPulse(90));
  Serial.printf("  %s 回到中位 90°\n\n", name);
  delay(500);
}

void loop() {
  sweepServo(12, "嘴巴 Mouth");
  sweepServo(13, "尾巴 Tail");
  sweepServo(14, "左耳 Left Ear");
  sweepServo(15, "右耳 Right Ear");

  Serial.println("--- 一轮扫描完成，5秒后重新开始 ---\n");
  delay(5000);
}
```

### 7. 多舵机同时控制 | Multi-Servo Simultaneous Control

```cpp
// Day 02 - 多舵机同时控制
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

#define SERVOMIN  102
#define SERVOMAX  512

int angleToPulse(int angle) {
  return map(constrain(angle, 0, 180), 0, 180, SERVOMIN, SERVOMAX);
}

// 同时设置多个舵机角度
void setServos(int mouth, int tail, int leftEar, int rightEar) {
  pwm.setPWM(12, 0, angleToPulse(mouth));
  pwm.setPWM(13, 0, angleToPulse(tail));
  pwm.setPWM(14, 0, angleToPulse(leftEar));
  pwm.setPWM(15, 0, angleToPulse(rightEar));
}

void setup() {
  Serial.begin(115200);
  Wire.begin(6, 7);
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(50);
  delay(100);

  // 初始位置：全部回到中位
  setServos(90, 90, 90, 90);
  Serial.println("所有舵机已回到中位");
}

void loop() {
  // "开心" 表情姿态
  Serial.println("表情: 开心 Happy");
  setServos(120, 130, 60, 60);   // 嘴巴张开，尾巴右摆，两耳下垂
  delay(1000);

  // "好奇" 表情姿态
  Serial.println("表情: 好奇 Curious");
  setServos(80, 70, 110, 110);   // 嘴巴微张，尾巴左摆，两耳竖起
  delay(1000);

  // "惊讶" 表情姿态
  Serial.println("表情: 惊讶 Surprised");
  setServos(140, 50, 130, 130);  // 嘴巴大张，尾巴大幅摆动
  delay(1000);

  // 回到中位
  Serial.println("回到中位 Neutral");
  setServos(90, 90, 90, 90);
  delay(2000);
}
```

## 拓展练习 | Extension Exercises

### 8. STS3032总线舵机简介 | STS3032 Bus Servo Introduction

STS3032是飞特（Feetech）的串行总线舵机，与普通PWM舵机不同：

The STS3032 is a Feetech serial bus servo. Unlike standard PWM servos:

```
特性对比:
┌────────────────┬────────────────┬─────────────────┐
│     特性       │   普通PWM舵机  │   STS3032总线舵机│
├────────────────┼────────────────┼─────────────────┤
│ 通信方式       │ PWM脉冲宽度    │ TTL串行通信      │
│ 接线数量       │ 3线(每舵机)    │ 3线(全部串联)    │
│ 通信速率       │ N/A            │ 最高1Mbps        │
│ 反馈           │ 无             │ 位置/温度/负载   │
│ 地址           │ 无             │ 可配置ID(0-253)  │
│ 扭矩           │ 一般           │ 大               │
└────────────────┴────────────────┴─────────────────┘
```

STS3032通信协议要点：
- TTL半双工串行通信
- 默认波特率: 1000000 (1Mbps)
- 数据格式: 8N1 (8数据位, 无校验, 1停止位)
- 每个舵机有唯一ID，可串联在一条总线上
- 支持读取位置、温度、负载等反馈信息

```cpp
// Day 02 - STS3032 通信示例 (概念性代码)
// 实际项目中使用 SCServo 库

#include <SCServo.h>

SMS_STS st;  // STS系列舵机对象

// XIAO ESP32S3 UART引脚
#define RXD2 D8   // UART RX
#define TXD2 D9   // UART TX

void setup() {
  Serial.begin(115200);

  // 初始化串口1用于STS3032通信
  Serial1.begin(1000000, SERIAL_8N1, RXD2, TXD2);
  st.pSerial = &Serial1;

  Serial.println("=== STS3032 总线舵机测试 ===");

  // 读取舵机1的位置
  int pos = st.ReadPos(1);
  Serial.printf("舵机1 当前位置: %d\n", pos);
}

void loop() {
  // 控制舵机1在0-4095范围内运动
  st.WritePosEx(1, 0, 2000, 50);    // ID=1, 位置=0, 速度=2000, 加速度=50
  delay(2000);
  st.WritePosEx(1, 2048, 2000, 50);  // 移动到中位
  delay(2000);
  st.WritePosEx(1, 4095, 2000, 50);  // 移动到最大位置
  delay(2000);
}
```

### 练习任务 | Practice Tasks

1. **I2C扫描器**: 编写I2C扫描程序，确认PCA9685地址正确

```cpp
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(6, 7);
  Serial.println("=== I2C 设备扫描 ===");

  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("发现设备 at 0x%02X\n", addr);
    }
  }
  Serial.println("扫描完成");
}

void loop() {}
```

2. **舵机校准**: 找出每个舵机的实际最小/最大角度范围
3. **平滑运动**: 使用 `sin()` 函数实现舵机的平滑往复运动

## 今日总结 | Day Summary

- 认识了桌面宠物项目的所有硬件组件
- 理解了I2C通信协议的工作原理
- 学会了使用PCA9685驱动板控制舵机
- 掌握了舵机PWM控制理论（50Hz, 0.5-2.5ms脉冲范围）
- 完成了舵机扫描和多舵机同时控制程序
- 了解了STS3032总线舵机的基本概念和通信方式
- **明日预告**: ST7789屏幕驱动与图形显示
