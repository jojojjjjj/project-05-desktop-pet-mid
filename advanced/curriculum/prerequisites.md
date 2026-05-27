# 前置知识（进阶方向）| Prerequisites (Advanced Track)

## 概述 | Overview

进阶方向在基础方向之上增加了 **Arduino 框架开发** 和 **Python 后端集成**，要求学员具备更强的编程能力和系统思维。本方向将使用 Arduino IDE 开发 ESP32 固件，并搭建 Python 后端服务器实现多模态 AI 功能。

The advanced track builds on the base track by adding **Arduino framework development** and **Python backend integration**. Students need stronger programming skills and systems thinking. This track uses Arduino IDE for ESP32 firmware and builds a Python backend server for multimodal AI features.

**强烈建议但非必须**：完成基础方向（Track A）的学习后再进入进阶方向。如果你有足够的编程和电子基础，也可以直接开始进阶方向。

**Recommended but not required**: Complete the base track (Track A) before starting the advanced track. If you have sufficient programming and electronics background, you may start directly.

---

## 必备基础 | Required Knowledge

### 1. Arduino IDE 基础

进阶方向使用 Arduino 框架（而非 ESP-IDF）开发 ESP32 固件，因此你需要熟悉 Arduino IDE 的基本操作。

The advanced track uses the Arduino framework (not ESP-IDF) for ESP32 firmware, so you must be comfortable with Arduino IDE basics.

| 知识点 | 要求程度 | 说明 |
|--------|----------|------|
| Arduino IDE 安装与配置 | 熟练 | 安装 IDE、配置开发板管理器 URL |
| 开发板管理器使用 | 熟练 | 添加 ESP32 开发板支持包 |
| 库管理器使用 | 熟练 | 搜索、安装、更新 Arduino 库 |
| 串口监视器 | 熟练 | 查看调试输出、设置波特率 |
| sketch 基本结构 | 熟练 | setup() 和 loop() 的概念 |
| 编译与上传 | 熟练 | 选择正确的开发板和端口 |

**推荐学习资源：**
- [Arduino 官方入门指南](https://www.arduino.cc/en/Guide) -- 英文，权威
- [Arduino 菜鸟教程](https://www.runoob.com/arduino/arduino-tutorial.html) -- 中文，适合入门
- B站搜索 "Arduino 入门教程" -- 视频学习

### 2. C/C++ 基础

与基础方向相同，但进阶方向需要更扎实的 C/C++ 基础，因为 Arduino 框架使用的是 C++ 子集。

Same as the base track, but the advanced track requires a more solid C/C++ foundation since Arduino uses a C++ subset.

| 知识点 | 要求程度 | 学习建议 |
|--------|----------|----------|
| 变量和数据类型 | 熟练 | int, char, float, uint8_t, uint16_t, String |
| 条件判断 | 熟练 | if/else, switch/case |
| 循环 | 熟练 | for 循环, while 循环 |
| 函数定义 | 熟练 | 函数声明、参数传递、返回值 |
| 指针 | 基本了解 | 指针概念、取地址&、解引用* |
| 结构体与类 | 基本了解 | struct 和 class 的基本概念 |
| 数组 | 熟练 | 一维数组、字符数组（字符串） |
| 头文件 | 了解即可 | .h 和 .cpp 文件的关系、#include |
| 预处理器宏 | 了解即可 | #define, #ifdef, #ifndef |

### 3. Python 基础（比基础方向更重要）

进阶方向的核心差异之一是使用 Python 搭建后端服务，因此 Python 能力比基础方向更为关键。

Python is a core differentiator in the advanced track. You will build a Python backend server, so Python proficiency is more important than in the base track.

| 知识点 | 要求程度 | 说明 |
|--------|----------|------|
| 变量和数据类型 | 熟练 | str, int, float, bool, list, dict |
| 函数定义 | 熟练 | def、参数、返回值、*args/**kwargs |
| 模块导入 | 熟练 | import、from...import、pip 安装第三方库 |
| 文件操作 | 基本了解 | open/read/write、with 语句 |
| 异常处理 | 基本了解 | try/except/finally |
| 类与对象 | 基本了解 | class、__init__、方法定义 |
| 虚拟环境 | 必须掌握 | python -m venv、激活 venv |
| pip 包管理 | 必须掌握 | pip install、requirements.txt |
| JSON 处理 | 熟练 | json.loads/dumps |
| HTTP 请求 | 基本了解 | requests 库或 httpx 库 |
| 异步编程 | 了解即可 | async/await 基本概念（FastAPI 使用） |

**推荐学习资源：**
- [Python 菜鸟教程](https://www.runoob.com/python3/python3-tutorial.html) -- 中文，系统学习
- [Python 官方教程](https://docs.python.org/zh-cn/3/tutorial/) -- 官方中文文档
- B站搜索 "Python 快速入门" -- 视频学习

### 4. 基本电路知识

与基础方向相同。

Same as the base track.

| 知识点 | 要求程度 | 说明 |
|--------|----------|------|
| 电压/电流/电阻 | 基本了解 | V=IR，串并联 |
| 直流电 vs 交流电 | 了解即可 | 开发板都是直流 5V/3.3V |
| 正负极 | 必须知道 | 接反可能烧坏芯片 |
| 通信协议概念 | 基本了解 | I2C、SPI、I2S 的基本概念（见下方加分项） |

### 5. 计算机基本操作

- 能够使用命令行（CMD/PowerShell/Terminal）
- 了解文件路径的概念（绝对路径 vs 相对路径）
- 能够安装软件和配置环境变量
- 有 VS Code 使用经验更佳
- 了解基本的网络概念（IP 地址、端口、HTTP）

### 6. 英文阅读能力

- 能够阅读简单的英文技术文档和错误信息
- 能够使用英文关键词搜索技术问题（Arduino 库文档以英文为主）
- 能够阅读 Python 库的 API 文档

---

## 加分项 | Nice-to-Have (Not Required)

以下知识不是必需的，但有基础会学得更快：

The following are not required but will help you progress faster:

| 知识 | 加分点 |
|------|--------|
| 基础方向（Track A）完成 | 硬件焊接经验、ESP32 基本概念已有基础 |
| I2C/SPI/I2S 协议理解 | 理解传感器通信、屏幕驱动、音频接口的工作原理 |
| Arduino ESP32 开发经验 | 已有 ESP32 + Arduino IDE 的项目经验 |
| Wi-Fi 网络基础 | 理解 SSID、IP、端口、HTTP/WebSocket 协议 |
| Python Web 框架经验 | FastAPI、Flask 等 Web 框架使用经验 |
| Git 基础 | 能够使用版本管理代码 |
| 焊接经验 | 有过电烙铁使用经验 |
| Linux 命令行 | 在 Linux/Mac 下开发体验更好 |
| API 调用经验 | 调用过大模型 API（如 OpenAI、通义千问） |

### 通信协议概念补充 | Communication Protocol Concepts

进阶方向会用到以下通信协议，提前了解有助于理解硬件工作原理：

The advanced track uses these communication protocols. Understanding them beforehand helps:

| 协议 | 用途 | 关键概念 |
|------|------|----------|
| I2C | 传感器通信（BMI270）、音频编解码器（ES8311） | 两线制（SDA/SCL）、地址寻址、主从模式 |
| SPI | 屏幕驱动（ST7789） | 四线制（MOSI/MISO/CLK/CS）、高速传输 |
| I2S | 音频数据传输 | 数字音频接口、位时钟/字选择/数据线 |
| UART/Serial | 调试输出和固件烧录 | 串口通信、波特率 |
| DVP | 摄像头接口（OV2640） | 并行数字视频端口、像素时钟 |

---

## 硬件要求 | Hardware Requirements

### 必备设备
- **笔记本电脑**：Windows 10/11（推荐），macOS 或 Linux 也可
- **USB Type-C 数据线**：用于连接 ESP32-S3 开发板（需支持数据传输，非仅充电线）

### 推荐设备
- **USB 转串口模块**：CP2102 或 CH340（备用调试工具）
- **外接显示器**：便于一边看教程一边操作

### 课程提供
- ESP-SparkBot 套件（包含 3 块 PCB + 所有元器件 + 3D 打印外壳）
- 焊接工具（电烙铁、焊锡、助焊剂、镊子、吸锡器等）
- 万用表

---

## 自学检查清单 | Self-Study Checklist

在课程开始之前，请尝试完成以下练习：

Before the course begins, try completing these exercises:

### 检查 1：Arduino IDE 基础

```cpp
// 请理解以下 Arduino sketch 的每一行代码
// Understand every line of this Arduino sketch

void setup() {
    Serial.begin(115200);  // 初始化串口通信
    pinMode(LED_BUILTIN, OUTPUT);  // 设置内置 LED 引脚为输出模式
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);  // 点亮 LED
    delay(1000);                      // 等待 1 秒
    digitalWrite(LED_BUILTIN, LOW);   // 熄灭 LED
    delay(1000);                      // 等待 1 秒
}
```

**验证步骤：**
1. 安装 Arduino IDE
2. 在开发板管理器中添加 ESP32 支持
3. 编译并上传以上 sketch 到 ESP32
4. 观察 LED 闪烁

### 检查 2：C/C++ 基础

```cpp
// 请理解以下代码，尝试解释每一行的作用
// Understand this code and explain each line

struct ServoState {
    int angle;
    int speed;
    bool isMoving;
};

void moveServo(ServoState* state, int targetAngle) {
    state->isMoving = true;
    state->angle = targetAngle;
    Serial.print("Moving to: ");
    Serial.println(targetAngle);
    state->isMoving = false;
}

void setup() {
    Serial.begin(115200);
    ServoState myServo = {0, 50, false};
    moveServo(&myServo, 90);
    Serial.print("Current angle: ");
    Serial.println(myServo.angle);
}

void loop() {}
```

### 检查 3：Python 基础

```python
# 请运行以下代码并理解输出结果
# Run this code and understand the output

import json

def process_sensor_data(data: dict) -> dict:
    """处理传感器数据，返回处理结果"""
    result = {
        "temperature": data.get("temp", 0) * 1.0,
        "humidity": data.get("humi", 0) * 1.0,
        "status": "normal"
    }
    if result["temperature"] > 30:
        result["status"] = "hot"
    return result

# 模拟传感器数据
raw_data = '{"temp": 28.5, "humi": 65.2}'
parsed = json.loads(raw_data)
processed = process_sensor_data(parsed)
print(json.dumps(processed, indent=2, ensure_ascii=False))
```

### 检查 4：Python 虚拟环境操作

```bash
# 请在命令行中完成以下操作
# Complete these operations in the command line

# Windows:
python -m venv myproject_env
myproject_env\Scripts\activate
pip install requests
python -c "import requests; print('requests OK')"
deactivate

# macOS/Linux:
python3 -m venv myproject_env
source myproject_env/bin/activate
pip install requests
python -c "import requests; print('requests OK')"
deactivate
```

### 检查 5：电路基础概念

```
请回答以下问题：
1. I2C 通信需要几根信号线？分别叫什么？
   答案：2 根 -- SDA（数据线）和 SCL（时钟线）

2. SPI 通信相比 I2C 的优势是什么？
   答案：速度更快（可达几十 MHz vs I2C 的 400KHz），适合屏幕等大数据量传输

3. I2S 是用来传输什么类型数据的？
   答案：数字音频数据（PCM 格式）

4. ESP32-S3 的 USB 接口可以用来做什么？
   答案：供电、串口调试、固件烧录、USB-OTG 功能
```

### 检查 6：Wi-Fi 网络基础

```
请回答以下问题：
1. HTTP 和 WebSocket 的区别是什么？
   答案：HTTP 是请求-响应模式，WebSocket 是全双工持久连接

2. 什么是 API Key？为什么调用大模型 API 需要它？
   答案：API Key 是身份认证凭证，用于计费和访问控制

3. 本地服务器（localhost）和远程服务器的区别是什么？
   答案：localhost 是本机地址（127.0.0.1），远程服务器通过网络访问
```

---

## 如果你还没有准备好 | If You're Not Ready Yet

不要担心！以下是加速学习的建议：

Don't worry! Here are suggestions to catch up:

1. **Arduino 零基础**：花 1-2 天学习 Arduino IDE 基本操作和 C++ 语法基础（推荐 B站 "太极创客" Arduino 教程）
2. **Python 零基础**：花 2-3 天集中学习 Python 基础语法和 pip/venv 使用
3. **电路零基础**：花半天时间了解基本的电路概念（电压、电流、电阻）
4. **网络知识薄弱**：花 2-3 小时了解 HTTP 协议、IP 地址和端口的基本概念
5. **通信协议不熟悉**：不阻塞学习，课程中会详细讲解 I2C/SPI/I2S 的使用

---

## 推荐预习 | Recommended Pre-reading

在开始课程之前，建议先浏览以下资料（不要求全部理解，混个脸熟即可）：

Before starting, browse these materials (no need to fully understand, just get familiar):

- **ESP32 Arduino 核心文档**：[Arduino-ESP32 GitHub](https://docs.espressif.com/projects/arduino-esp32/en/latest/) -- 了解 Arduino 框架如何驱动 ESP32
- **Python FastAPI 教程**：[FastAPI 官方教程](https://fastapi.tiangolo.com/zh/tutorial/) -- 进阶方向后端使用的框架
- **通义千问 API 文档**：[DashScope API](https://help.aliyun.com/zh/model-studio/) -- AI 多模态 API 调用参考
- **Arduino 库管理指南**：[Arduino Library Manager](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries) -- 库安装方法

---

## 学习时间估算 | Time Estimate

| 学员水平 | 前置知识准备时间 |
|----------|-----------------|
| 有 Arduino + Python 经验 | 0-2 小时（浏览即可） |
| 有 Python 基础 + 简单电路知识 | 4-6 小时（补 Arduino IDE） |
| 有 C/C++ 基础但无 Python 经验 | 1-2 天（集中学 Python） |
| 有编程基础但无 Arduino/Python 经验 | 2-3 天 |
| 完全零基础 | 4-5 天集中学习 |

---

## 进阶方向 vs 基础方向对比 | Advanced vs Base Track Comparison

| 对比项 | 基础方向（Track A） | 进阶方向（Track B） |
|--------|---------------------|---------------------|
| 开发框架 | ESP-IDF | Arduino + Python |
| 编程语言 | C | C/C++ + Python |
| 开发工具 | VS Code + ESP-IDF 插件 | Arduino IDE + 任意 Python IDE |
| AI 集成方式 | ESP32 直接调 API | ESP32 + Python 后端转发 |
| 学习重点 | 嵌入式底层驱动 | 快速原型 + 全栈集成 |
| 适合人群 | 想深入嵌入式开发 | 想做完整产品体验 |
| 难度 | 较高（底层概念多） | 中等（框架封装度高） |

*最后更新：2026-05-27*
