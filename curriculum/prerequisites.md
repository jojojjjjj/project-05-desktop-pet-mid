# 前置知识 | Prerequisites

## 必备基础 | Required Knowledge

### 1. C语言基础
在开始本项目前，你应该掌握以下C语言基础知识：

Before starting, you should be comfortable with these C language basics:

| 知识点 | 要求程度 | 学习建议 |
|--------|----------|----------|
| 变量和数据类型 | 熟练 | int, char, float, uint8_t, uint16_t, uint32_t |
| 条件判断 | 熟练 | if/else, switch/case |
| 循环 | 熟练 | for循环, while循环 |
| 函数定义 | 熟练 | 函数声明、参数传递、返回值 |
| 指针 | 基本了解 | 指针概念、取地址&、解引用* |
| 结构体 | 基本了解 | struct定义和使用 |
| 数组 | 熟练 | 一维数组、字符数组（字符串） |
| 头文件 | 了解即可 | .h和.c文件的关系、#include |

**推荐学习资源：**
- [C语言菜鸟教程](https://www.runoob.com/cprogramming/c-tutorial.html) -- 中文，适合零基础
- [C语言中文网](http://c.biancheng.net/c/) -- 系统学习C语言
- B站搜索 "C语言入门教程" -- 视频学习

### 2. Python基础（用于AI API脚本）
仅需基础的Python知识，用于理解和调试AI API调用：

| 知识点 | 要求程度 | 说明 |
|--------|----------|------|
| 变量和数据类型 | 基本了解 | str, dict, list |
| 函数调用 | 基本了解 | 会调用现成的函数即可 |
| JSON处理 | 了解即可 | json.loads/dumps |
| HTTP请求 | 了解即可 | requests库基本使用 |

### 3. 基本电路知识
| 知识点 | 要求程度 | 说明 |
|--------|----------|------|
| 电压/电流/电阻 | 基本了解 | V=IR，串并联 |
| 直流电 vs 交流电 | 了解即可 | 开发板都是直流5V/3.3V |
| 万用表使用 | 了解即可 | Day 2会详细教学 |
| 正负极 | 必须知道 | 接反可能烧坏芯片！ |

### 4. 计算机基本操作
- 能够使用命令行（CMD/PowerShell/Terminal）
- 了解文件路径的概念（绝对路径 vs 相对路径）
- 能够安装软件和配置环境变量
- 有VS Code使用经验更佳

### 5. 英文阅读能力
- 能够阅读简单的英文技术文档和错误信息
- 能够使用英文关键词搜索技术问题（ESP-IDF文档以英文为主）

## 加分项 | Nice-to-Have (Not Required)

以下知识不是必需的，但有基础会学得更快：

The following are not required but will help you progress faster:

| 知识 | 加分点 |
|------|--------|
| 焊接经验 | 有过电烙铁使用经验，Day 3-4会更轻松 |
| 3D打印经验 | 了解切片软件和3D打印机操作 |
| Arduino经验 | 了解嵌入式开发的基本概念 |
| Git基础 | 能够使用版本管理代码 |
| Linux命令行 | ESP-IDF在Linux/Mac下体验更好 |
| 其他编程语言经验 | 理解编程通用概念 |

## 硬件要求 | Hardware Requirements

### 必备设备
- **笔记本电脑**：Windows 10/11（推荐），macOS或Linux也可
- **USB Type-C数据线**：用于连接ESP32-S3开发板（需支持数据传输，非仅充电线）

### 推荐设备
- **USB转串口模块**：CP2102或CH340（备用调试工具）
- **外接显示器**：便于一边看教程一边操作

### 课程提供
- ESP-SparkBot套件（包含3块PCB + 所有元器件 + 3D打印外壳）
- 焊接工具（电烙铁、焊锡、助焊剂、镊子、吸锡器等）
- 万用表
- 热风枪（可选）

## 自学检查清单 | Self-Study Checklist

在Day 1之前，请尝试完成以下练习：

Before Day 1, try completing these exercises:

### 检查1：C语言基础
```c
// 请在脑海中（或在在线编译器中）运行以下代码，理解每一行的含义
#include <stdio.h>

int main() {
    char name[] = "SparkBot";
    int version = 1;
    
    if (version > 0) {
        printf("%s version %d is ready!\n", name, version);
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Beep #%d!\n", i + 1);
    }
    
    return 0;
}
```

### 检查2：指针基本概念
```c
// 请理解以下代码，尝试画出内存示意图
int value = 42;
int *ptr = &value;   // ptr指向value的地址
*ptr = 100;          // 通过指针修改value的值
printf("value = %d\n", value);  // 输出: value = 100
```

### 检查3：命令行操作
```bash
# 请在命令行中完成以下操作
# Windows (PowerShell):
mkdir my_esp_project
cd my_esp_project
echo "Hello, ESP32!" > hello.txt
type hello.txt

# macOS/Linux:
mkdir my_esp_project
cd my_esp_project
echo "Hello, ESP32!" > hello.txt
cat hello.txt
```

### 检查4：电路基础概念
```
请回答以下问题：
1. 一个LED灯串联一个220欧姆电阻接3.3V，电流大约是多少？（提示：V=IR）
   答案：约15mA（3.3V / 220Ω = 0.015A）
2. USB提供的电压是几伏？ESP32-S3的工作电压是几伏？
   答案：USB 5V，ESP32-S3 3.3V
3. 为什么开发板上需要"降压模块"（DC-DC）？
   答案：将USB的5V转换为3.3V供ESP32-S3使用
```

## 如果你还没有准备好 | If You're Not Ready Yet

不要担心！以下是加速学习的建议：

Don't worry! Here are suggestions to catch up:

1. **C语言零基础**：花2-3天集中学习C语言基础语法（推荐B站翁恺C语言或小甲鱼C语言教程）
2. **电路零基础**：花半天时间了解基本的电路概念（电压、电流、电阻）
3. **命令行不熟悉**：花2-3小时学习基本的命令行操作
4. **安装有困难**：Day 1的课程会详细指导ESP-IDF环境配置

## 推荐预习 | Recommended Pre-reading

在开始课程之前，建议先浏览以下资料（不要求全部理解，混个脸熟即可）：

- **ESP32-S3技术参考手册摘要**：[ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf) -- 浏览第1-3章了解芯片特性
- **ESP-IDF编程指南**：[ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/) -- 浏览"Get Started"章节
- **FreeRTOS简介**：搜索 "FreeRTOS 入门教程" -- 了解什么是任务、调度器

## 学习时间估算 | Time Estimate

| 学员水平 | 前置知识准备时间 |
|----------|-----------------|
| 有嵌入式开发经验 | 0-2小时（浏览即可） |
| 有C语言基础+简单电路知识 | 4-8小时 |
| 有编程基础但无C语言经验 | 1-2天 |
| 完全零基础 | 3-4天集中学习 |

*最后更新：2026-05-27*
