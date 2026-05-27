# 课程总览 | Curriculum Overview

## 项目名称 | Project Name
AICat Advanced | AICat 进阶版 —— 多模态 AI 桌面机器猫

## 项目定位 | Project Positioning
本项目是一个面向高中生的进阶暑期硬件实践课程，为期12天（约2周半），每天6-8小时。学员将在桌面宠物基础版之上，使用 Seeed XIAO ESP32S3 Sense 开发板、STS3032 总线舵机（四足行走）、ST7789 屏幕（表情动画）、MAX98357A I2S 扬声器等硬件，结合 Python 后端与通义千问 3.5 Omni 多模态大模型，亲手构建一台能听、能看、能说话、能走路的多模态 AI 桌面猫。

This is an advanced 12-day summer hardware practicum for high school students (6-8 hours per day). Building on the base desktop pet project, students will use the Seeed XIAO ESP32S3 Sense, STS3032 bus servos (4-legged walking), ST7789 screen (facial expressions), MAX98357A I2S speaker, and a Python backend powered by Qwen 3.5 Omni to build a multimodal AI desktop cat that can listen, see, speak, and walk.

## 学习目标 | Learning Objectives

### 知识目标 | Knowledge Objectives
1. 掌握 Arduino IDE 开发环境与 ESP32-S3 嵌入式编程基础
2. 理解 I2C、SPI、I2S、UART 等通信协议在多外设协同中的原理与应用
3. 理解 PCA9685 PWM 舵机驱动与 STS3032 总线舵机的控制原理及行走步态规划
4. 掌握 LittleFS 文件系统在嵌入式设备中的图像存储与帧动画原理
5. 理解 WebSocket 实时通信协议及其在硬件-AI 后端架构中的作用
6. 了解多模态大模型 API 的调用方式（语音+文本输入、语音合成输出）

### 技能目标 | Skill Objectives
1. 能够独立搭建 Arduino IDE 开发环境并完成 ESP32-S3 固件编译烧录
2. 能够使用多协议总线（I2C/SPI/I2S/UART）同时驱动屏幕、摄像头、音频和舵机
3. 能够设计并实现表情帧动画系统（LittleFS + JPEG 解码 + 嘴型同步）
4. 能够编写四足机器人的行走步态控制代码
5. 能够搭建 Python 后端服务，集成 ASR（语音识别）与大模型 API
6. 能够完成多模块系统集成调试，并独立进行项目演示和技术表达

### 素养目标 | Competency Objectives
1. 培养多协议、多外设复杂系统的集成工程思维
2. 培养软硬件协同调试的耐心与排错能力（串口调试、逻辑分析）
3. 培养项目分阶段推进的计划意识（硬件 → 固件 → 后端 → 集成）
4. 培养团队协作与任务分工的能力（前端固件 / 后端服务 / 机械组装）
5. 培养技术文档撰写和项目展示的表达能力

## 课程结构 | Course Structure

### 第一阶段：开发环境搭建与硬件组装（Day 1-4）
**主题：Arduino IDE 环境、XIAO ESP32S3 Sense、舵机系统、屏幕与音频硬件**

在这一阶段，学员将搭建 Arduino IDE 开发环境，熟悉 XIAO ESP32S3 Sense 开发板的各项功能，学习 PCA9685 PWM 驱动和 STS3032 总线舵机的控制方法，完成 ST7789 屏幕驱动和 MAX98357A 音频测试，并进行整机硬件组装与接线验证。完成本阶段后，学员将拥有一台硬件完整、各模块可独立驱动的 AICat 原型。

Students set up the Arduino IDE environment, explore the XIAO ESP32S3 Sense board, learn PCA9685 PWM and STS3032 bus servo control, complete ST7789 screen driving and MAX98357A audio testing, and perform full hardware assembly and wiring verification. By the end of this phase, students will have a fully assembled AICat prototype with all modules individually functional.

| 天数 | 主题 | 核心内容 |
|------|------|----------|
| Day 1 | Arduino IDE 与 XIAO ESP32S3 Sense 入门 | 开发环境搭建、开发板介绍、第一个 Sketch（Blink + 串口通信） |
| Day 2 | 硬件组件总览与舵机系统 | 组件功能梳理、PCA9685 PWM 驱动基础舵机控制、STS3032 总线舵机介绍 |
| Day 3 | ST7789 屏幕驱动与图形基础 | SPI 通信驱动屏幕、简单图形绘制、表情显示概念 |
| Day 4 | 硬件组装与音频测试 | 整机组装、接线验证、MAX98357A I2S 音频播放测试 |

### 第二阶段：核心功能开发（Day 5-8）
**主题：摄像头流、音频录制、舵机集成、表情动画系统**

这一阶段是项目的核心，学员将实现 OV2640 摄像头图像采集与 WebSocket 视频流传输，完成麦克风音频录制，将 PCA9685（嘴部/尾巴/耳朵）与 STS3032（四足行走）舵机系统集成，并构建完整的表情动画引擎（LittleFS 文件系统、JPEG 帧解码、嘴型同步）。完成本阶段后，AICat 将具备看、听、表达、行走的能力。

This is the core phase. Students implement OV2640 camera capture and WebSocket video streaming, microphone audio recording, PCA9685 (mouth/tail/ears) and STS3032 (quadruped walking) servo integration, and a full facial expression animation engine (LittleFS filesystem, JPEG frame decoding, mouth synchronization). After this phase, AICat will be able to see, hear, express, and walk.

| 天数 | 主题 | 核心内容 |
|------|------|----------|
| Day 5 | 摄像头流与 WebSocket 基础 | OV2640 图像采集、WiFi 连接、WebSocket 实时视频流 |
| Day 6 | 麦克风与音频录制 | I2S 深入、音频数据采集与缓存、PCM 格式处理 |
| Day 7 | 舵机系统集成 | PCA9685 嘴部/尾巴/耳朵控制、STS3032 四足行走步态 |
| Day 8 | 表情动画系统 | LittleFS 文件系统、JPEG 帧动画引擎、嘴型同步机制 |

### 第三阶段：AI 集成与项目展示（Day 9-12）
**主题：Python 后端、通义千问 API、全流水线集成、项目演示**

最后阶段涵盖 Python 后端服务搭建（Flask/FastAPI + WebSocket 服务器）、ASR 语音识别集成、通义千问 3.5 Omni 多模态 API 对接（语音+文本输入、TTS 语音合成输出），以及全流水线集成（语音输入 → ASR → AI 推理 → TTS → 扬声器播放 + 表情同步）。学员最终将完成 Web 控制面板并进行项目展示。

The final phase covers the Python backend (Flask/FastAPI + WebSocket server), ASR integration, Qwen 3.5 Omni multimodal API integration (audio+text input, TTS output), and full pipeline integration (voice in → ASR → AI → TTS → speaker + expression sync). Students complete a web control panel and deliver a final project demo.

| 天数 | 主题 | 核心内容 |
|------|------|----------|
| Day 9 | Python 后端搭建 | Flask/FastAPI 框架、WebSocket 服务器、ASR 语音识别集成 |
| Day 10 | 通义千问 API 集成 | DashScope API 调用、多模态输入（文本+音频）、TTS 语音合成 |
| Day 11 | 全流水线集成 | 语音输入 → ASR → AI → TTS → 扬声器，表情同步联调 |
| Day 12 | 完善与项目展示 | Web 控制面板、系统优化、项目演示与汇报 |

## 每日时间安排 | Daily Schedule Template

| 时间 | 活动 | 说明 |
|------|------|------|
| 09:00 - 09:15 | 晨间回顾 | 回顾昨日作业，解答疑问 |
| 09:15 - 10:30 | 知识讲解 | 今日核心概念和原理 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | 跟随教程完成代码编写/硬件操作 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | 将所学应用到 AICat 项目 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 个性化定制和功能拓展 |
| 16:30 - 17:00 | 总结分享 | 今日收获总结、小组分享 |

## 教学方法 | Teaching Methods

1. **项目驱动**：所有知识点都围绕 AICat 项目展开，学以致用 / All knowledge points are anchored to the AICat project, learning by doing
2. **分步引导**：每个任务都提供详细的步骤说明和代码示例，降低认知负荷 / Each task comes with detailed step-by-step instructions and code examples to reduce cognitive load
3. **即时反馈**：每完成一个任务都能在硬件上看到实际效果（舵机转动、屏幕表情、声音播放） / Immediate feedback: every completed task produces visible results on hardware
4. **软硬件协同**：嵌入式固件与 Python 后端并行开发，培养全栈工程视野 / Parallel firmware and backend development fosters full-stack engineering perspective
5. **同伴学习**：集成调试环节鼓励学员之间互相帮助、分工协作 / Integration debugging sessions encourage peer collaboration and task sharing
6. **自主探索**：每日作业鼓励个性化定制（自定义步态、创意表情、独特对话风格） / Daily assignments encourage personalization and creative experimentation

## 前置要求 | Prerequisites
详见 [../../curriculum/prerequisites.md](../../curriculum/prerequisites.md)（基础版前置要求同样适用于进阶版）

### 进阶版额外建议 | Additional Recommendations for Advanced Track
- 完成桌面宠物基础版课程，或具备同等 ESP32 嵌入式开发经验
- 基本的 Python 编程能力（变量、函数、字典、HTTP 请求）
- 了解 Arduino IDE 的基本使用
- 对机器人运动学和 AI 对话系统有好奇心

## 评分标准 | Grading Rubric
详见 [grading-rubric.md](grading-rubric.md)

## 作业说明 | Assignments
详见 [assignments.md](assignments.md)

## 技术栈总览 | Tech Stack Overview

| 层级 | 组件 | 技术 |
|------|------|------|
| 主控 | Seeed XIAO ESP32S3 Sense | Arduino IDE, ESP32-S3 双核 240MHz |
| 视觉 | OV2640 摄像头 (板载) | DVP 接口, JPEG 输出 |
| 显示 | ST7789 SPI 屏幕 | 240x240, LittleFS 帧动画 |
| 音频输入 | 板载麦克风 | I2S PCM 采集 |
| 音频输出 | MAX98357A I2S 功放 | I2S PCM 播放 |
| 舵机 (表情) | PCA9685 PWM 驱动 | I2C, 嘴部/尾巴/耳朵 |
| 舵机 (行走) | STS3032 总线舵机 x4 | UART TTL, 四足步态 |
| 通信 | WiFi + WebSocket | ESP32-S3 WiFi, 实时双向通信 |
| 后端 | Python (Flask/FastAPI) | WebSocket 服务器, ASR, TTS |
| AI | 通义千问 3.5 Omni | DashScope API, 多模态输入输出 |

*最后更新：2026-05-27*
