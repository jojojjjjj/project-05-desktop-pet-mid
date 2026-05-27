# 课程总览 | Curriculum Overview

## 项目名称 | Project Name
ESP-SparkBot | ESP-SparkBot AI桌面机器人

## 项目定位 | Project Positioning
本项目是一个面向高中生的暑期硬件实践课程，为期12天（约2周半），每天6-8小时。学员将从零开始，使用ESP32-S3芯片和ESP-IDF开发框架，亲手焊接、组装并编程一个具备AI对话、语音交互、天气显示、游戏娱乐等多功能的桌面机器人。

This is a 12-day summer hardware practicum for high school students (6-8 hours per day). Students will build an AI desktop robot from scratch using the ESP32-S3 chip and ESP-IDF framework, with hands-on soldering, assembly, and embedded C programming.

## 学习目标 | Learning Objectives

### 知识目标 | Knowledge Objectives
1. 掌握ESP-IDF开发环境和嵌入式C语言编程基础
2. 理解FreeRTOS实时操作系统的任务调度与通信机制
3. 掌握I2C、SPI、I2S、DVP等通信协议的原理和应用
4. 理解传感器驱动开发的基本方法
5. 了解WiFi、蓝牙、HTTP协议在嵌入式设备中的应用
6. 了解AI大模型API的调用和语音交互流水线

### 技能目标 | Skill Objectives
1. 能够独立搭建ESP-IDF开发环境并编译烧录固件
2. 能够使用万用表和示波器进行基本的硬件调试
3. 能够完成贴片元件的焊接和PCB组装
4. 能够阅读原理图并理解各模块间的连接关系
5. 能够编写嵌入式C代码驱动显示屏、摄像头、音频等外设
6. 能够进行项目演示和技术表达

### 素养目标 | Competency Objectives
1. 培养"硬件+软件"系统集成的工程思维
2. 培养独立调试和排错的能力（嵌入式调试需要更多耐心）
3. 培养项目管理和进度规划的意识
4. 培养安全操作意识（焊接安全、用电安全）
5. 培养技术文档撰写和表达能力

## 课程结构 | Course Structure

### 第一阶段：嵌入式开发环境搭建与硬件组装（Day 1-4）
**主题：ESP-IDF环境搭建、焊接基础、PCB组装、硬件调试**

在这一阶段，学员将搭建ESP-IDF开发环境，学习ESP32-S3芯片架构，掌握焊接技能，完成SparkBot三块PCB的焊接和整机组装。完成本阶段后，学员将拥有一台硬件完整的SparkBot机器人。

Students set up the ESP-IDF development environment, learn ESP32-S3 architecture, master soldering skills, and complete PCB assembly. By the end of this phase, students will have a fully assembled SparkBot robot.

### 第二阶段：核心功能开发与调试（Day 5-8）
**主题：GPIO编程、传感器驱动、显示屏GUI、摄像头与音频系统**

这一阶段是项目的核心，学员将学习如何用C语言驱动各种外设：通过I2C读取传感器数据、通过SPI驱动显示屏、使用LVGL开发GUI界面、驱动摄像头和音频编解码器。完成本阶段后，SparkBot将具备基本的感知和交互能力。

This is the core phase. Students learn to drive peripherals with C: I2C sensor reading, SPI display driving, LVGL GUI development, camera and audio codec integration. After this phase, SparkBot will have basic perception and interaction capabilities.

### 第三阶段：AI功能集成与项目展示（Day 9-12）
**主题：WiFi联网、AI大模型对话、系统集成、项目展示**

最后阶段涵盖WiFi联网获取天气数据、调用LLM API实现AI对话、语音识别与合成、系统稳定性优化和最终项目展示。学员将所有功能整合为一个完整的AI桌面机器人产品。

The final phase covers WiFi connectivity for weather data, LLM API integration for AI dialogue, speech recognition and synthesis, system optimization, and the final demo. Students integrate all features into a complete AI desktop robot.

## 每日时间安排 | Daily Schedule Template

| 时间 | 活动 | 说明 |
|------|------|------|
| 09:00 - 09:15 | 晨间回顾 | 回顾昨日作业，解答疑问 |
| 09:15 - 10:30 | 知识讲解 | 今日核心概念和原理 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | 跟随教程完成代码编写/硬件操作 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | 将所学应用到SparkBot项目 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 个性化定制和功能拓展 |
| 16:30 - 17:00 | 总结分享 | 今日收获总结、小组分享 |

## 教学方法 | Teaching Methods

1. **项目驱动**：所有知识点都围绕SparkBot项目展开，学以致用
2. **分步引导**：每个任务都提供详细的步骤说明和代码示例，降低认知负荷
3. **即时反馈**：每完成一个任务都能在硬件上看到实际效果（LED亮起、屏幕显示、声音播放）
4. **同伴学习**：硬件焊接和调试环节鼓励学员之间互相帮助、检查焊点
5. **自主探索**：每日作业鼓励个性化定制和创新尝试（自定义界面、创意功能）

## 前置要求 | Prerequisites
详见 [prerequisites.md](prerequisites.md)

## 评分标准 | Grading Rubric
详见 [grading-rubric.md](grading-rubric.md)

## 作业说明 | Assignments
详见 [assignments.md](assignments.md)

*最后更新：2026-05-27*
