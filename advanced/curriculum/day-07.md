# Day 07: 舵机系统集成与动作编排 | Servo System Integration & Motion Sequences

> **今日目标:**
> - 同时控制 PCA9685 多通道舵机（嘴、尾、耳）
> - 编写 STS3032 四足步态程序（走路、坐下）
> - 设计动作序列（emotion → action 映射）
> - 实现平滑的舵机运动
>
> **产出:** 猫咪能完成走路、坐下、摇尾巴、动耳朵等动作序列

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间回顾 | 音频处理与 VAD 回顾 |
| 09:15 - 10:30 | 知识讲解 | 多舵机协调、STS3032 总线协议、步态分析 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | PCA9685 多通道控制 + STS3032 单腿运动 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | 四足步态编程 + 情绪-动作映射 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 自定义动作编排、动作录制 |
| 16:30 - 17:00 | 总结分享 | 动作效果展示 |

---

## 上午: 多舵机协调控制 | Morning: Multi-Servo Coordination

### PCA9685 通道分配 | PCA9685 Channel Map

```
PCA9685 通道分配:
  CH12 → 嘴巴 (2g 微型舵机, 0°=闭, 30°=微张, 60°=大张)
  CH13 → 尾巴 (2g 微型舵机, 左右摆动)
  CH14 → 左耳 (MG90S, 前=警觉, 后=害怕)
  CH15 → 右耳 (MG90S, 前=警觉, 后=害怕)
```

### 平滑运动函数 | Smooth Motion

```cpp
void smoothMove(uint8_t channel, int fromAngle, int toAngle, int stepDelay) {
    int step = (toAngle > fromAngle) ? 1 : -1;
    for (int a = fromAngle; a != toAngle; a += step) {
        setServoAngle(channel, a);
        delay(stepDelay);
    }
    setServoAngle(channel, toAngle);
}

// 同时平滑移动多个舵机
void smoothMoveMulti(int *channels, int *targets, int count, int stepDelay) {
    int *current = new int[count];
    for (int i = 0; i < count; i++)
        current[i] = targets[i]; // 简化: 假设从目标位置开始

    bool moving = true;
    while (moving) {
        moving = false;
        for (int i = 0; i < count; i++) {
            if (current[i] != targets[i]) {
                current[i] += (targets[i] > current[i]) ? 1 : -1;
                setServoAngle(channels[i], current[i]);
                moving = true;
            }
        }
        delay(stepDelay);
    }
    delete[] current;
}
```

### STS3032 总线舵机 | STS3032 Bus Servos

STS3032 使用 TTL 串行通信，每条腿一个舵机：

```
STS3032 舵机 ID 分配:
  ID1 → 左前腿
  ID2 → 右前腿
  ID3 → 左后腿
  ID4 → 右后腿

通信: Serial1, 波特率 1000000
  TX → D6 (GPIO43)
  RX → D7 (GPIO44)
```

```cpp
#include <SCServo.h>

SMS_STS st;

void setupSTS() {
    Serial1.begin(1000000, SERIAL_8N1, 44, 43);
    st.pSerial = &Serial1;
}

// 移动腿到指定位置
void moveLeg(uint8_t id, int position, uint16_t speed) {
    st.MoveTo(id, position, speed, 0);
}
```

---

## 下午: 四足步态编程 | Afternoon: Quadruped Gait

### 基本步态分析 | Gait Analysis

```
走路步态 (Walk Gait) — 4个相位:

相位1: 左前+右后 抬起前移
     ┌─┐     ┌─┐
     │↑│     │ │
     └─┘     └─┘
     ┌─┐     ┌─┐
     │ │     │↑│
     └─┘     └─┘

相位2: 所有着地，身体前移
相位3: 右前+左后 抬起前移
相位4: 所有着地，身体前移

对角步态 (Trot): 对角线腿同时运动
```

### 步态代码 | Gait Code

```cpp
void walkGait(int steps) {
    for (int i = 0; i < steps; i++) {
        // 相位1: 左前(ID1) + 右后(ID4) 抬起
        moveLeg(1, 2000, 500);
        moveLeg(4, 2000, 500);
        delay(200);
        moveLeg(1, 1000, 500);
        moveLeg(4, 1000, 500);
        delay(200);

        // 相位3: 右前(ID2) + 左后(ID3) 抬起
        moveLeg(2, 2000, 500);
        moveLeg(3, 2000, 500);
        delay(200);
        moveLeg(2, 1000, 500);
        moveLeg(3, 1000, 500);
        delay(200);
    }
}

void sitDown() {
    // 前腿伸直，后腿弯曲
    moveLeg(1, 1500, 300);
    moveLeg(2, 1500, 300);
    moveLeg(3, 2500, 300);
    moveLeg(4, 2500, 300);
    delay(500);
}
```

### 情绪-动作映射 | Emotion-Action Mapping

```cpp
void expressEmotion(const char* emotion) {
    if (strcmp(emotion, "happy") == 0) {
        // 尾巴快速摆动
        for (int i = 0; i < 3; i++) {
            smoothMove(13, 60, 120, 15);  // 尾巴左
            smoothMove(13, 120, 60, 15);  // 尾巴右
        }
    } else if (strcmp(emotion, "sad") == 0) {
        // 耳朵耷拉
        smoothMove(14, 90, 30, 20);  // 左耳后
        smoothMove(15, 90, 150, 20); // 右耳后
    } else if (strcmp(emotion, "surprised") == 0) {
        // 耳朵竖起 + 嘴巴张开
        smoothMove(14, 90, 150, 10);
        smoothMove(15, 90, 30, 10);
        smoothMove(12, 0, 60, 10);   // 嘴巴张开
    }
}
```

---

## 拓展练习 | Extension Exercises

### 练习1: 跳跃动作
设计一个让猫"跳起来"的步态序列（先蹲下再快速伸展）。

### 练习2: 串口控制
通过串口输入命令（"happy"、"sad"、"walk"、"sit"），触发对应的动作序列。

---

## 今日总结 | Day Summary

| 知识点 | 掌握程度 |
|--------|----------|
| PCA9685 多通道 | 能同时控制 4 个舵机 |
| STS3032 总线舵机 | 能通过串行命令控制四条腿 |
| 步态编程 | 理解对角步态的 4 相位设计 |
| 平滑运动 | 掌握逐步逼近的平滑算法 |
| 情绪映射 | 能将情绪文本转化为动作序列 |

*最后更新：2026-05-27*
