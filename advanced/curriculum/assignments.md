# 作业说明（进阶方向）| Assignments Overview (Advanced Track)

## 作业体系 | Assignment Structure

进阶方向的作业按三个阶段组织，贯穿整个课程周期。与基础方向不同，进阶方向更侧重于 **Arduino 固件开发** 和 **Python 后端集成** 的能力培养。

Advanced track assignments are organized in three phases throughout the course. Unlike the base track, the advanced track emphasizes **Arduino firmware development** and **Python backend integration** skills.

| 类型 | 提交时间 | 说明 |
|------|----------|------|
| 阶段一作业 | 第一阶段结束 | 硬件搭建文档、Arduino 环境验证、blink 变体程序 |
| 阶段二作业 | 第二阶段结束 | 舵机控制代码、屏幕动画代码、摄像头流媒体 Demo |
| 阶段三作业 | 第二阶段结束 | Python 后端集成代码、AI 对话 Demo、最终项目报告 |
| 周进度汇报 | 每周末 | 书面总结本周进展、问题和计划 |
| 最终展示 | 课程最后一天 | 项目演示 + 技术答辩 |

---

## 阶段一作业 | Phase 1 Assignments

**主题：硬件搭建与环境验证 | Hardware Setup & Environment Verification**

### 作业 1.1：硬件搭建文档 | Hardware Setup Documentation

**要求：**
- 记录完整的硬件搭建过程，包含文字说明和照片
- Document the complete hardware setup process with text descriptions and photos

**提交内容：**
- [ ] 焊接过程照片（至少 5 张：焊接前 PCB、焊接中、焊接后正面、焊接后背面、万用表测试）
- [ ] 接线图文档（标注各模块连接方式：屏幕、舵机、摄像头、麦克风、喇叭）
- [ ] 万用表测试记录（各电压测试点实测值表格）
- [ ] 首次上电测试结果（串口输出截图）

**评分标准：**
| 评价项 | 分值 | 说明 |
|--------|------|------|
| 照片质量与完整性 | 30 分 | 清晰可见、覆盖所有关键步骤 |
| 接线图准确性 | 30 分 | 标注正确、引脚对应无误 |
| 测试数据完整性 | 20 分 | 各测试点数据记录完整 |
| 文档排版与可读性 | 20 分 | 结构清晰、中英双语 |

### 作业 1.2：Arduino Blink 变体程序 | Arduino Blink Variants

**要求：**
- 编写至少 3 个不同功能的 blink 变体程序
- Write at least 3 blink variant programs with different behaviors

**提交内容：**
- [ ] `blink_basic.ino` -- 基本 LED 闪烁（理解 setup/loop 结构）
- [ ] `blink_button.ino` -- 按键控制 LED（理解 GPIO 输入）
- [ ] `blink_pwm.ino` -- PWM 呼吸灯效果（理解模拟输出）
- [ ] `blink_serial.ino` -- 串口控制闪烁频率（加分项：理解串口通信）

**评分标准：**
| 评价项 | 分值 | 说明 |
|--------|------|------|
| 基本功能实现 | 40 分 | 至少完成 3 个变体 |
| 代码质量 | 30 分 | 注释充分、命名规范、结构清晰 |
| 串口变体（加分） | 20 分 | 实现串口控制功能 |
| 创意变体（加分） | 10 分 | 额外的创意 blink 效果 |

**Arduino 代码基本要求：**
- 代码能够通过 Arduino IDE 编译无错误
- 能够上传到 ESP32-S3 并正常运行
- 关键代码行有中文或英文注释
- 变量和函数命名有意义（非 a, b, c）
- 使用 #define 或 const 定义常量引脚号

---

## 阶段二作业 | Phase 2 Assignments

**主题：硬件驱动与多媒体功能 | Hardware Drivers & Multimedia Features**

### 作业 2.1：舵机控制代码 | Servo Control Code

**要求：**
- 实现舵机的精确角度控制和运动序列
- Implement precise angle control and motion sequences for servos

**提交内容：**
- [ ] `servo_basic.ino` -- 舵机基本控制（0-180 度旋转）
- [ ] `servo_smooth.ino` -- 舵机平滑运动（缓动函数）
- [ ] `servo_sequence.ino` -- 运动序列（如点头、摇头、挥手）
- [ ] 舵机运动视频（录制 30 秒运动效果）

**评分标准：**
| 评价项 | 分值 | 说明 |
|--------|------|------|
| 基本角度控制 | 30 分 | 精确定位到指定角度 |
| 平滑运动 | 30 分 | 运动流畅无抖动 |
| 运动序列编排 | 25 分 | 至少实现 3 组动作序列 |
| 代码注释 | 15 分 | 缓动算法有原理解释 |

### 作业 2.2：屏幕动画代码 | Screen Animation Code

**要求：**
- 在 SPI LCD 屏幕上实现动画效果
- Implement animation effects on the SPI LCD screen

**提交内容：**
- [ ] `display_basic.ino` -- 基本文字和图形显示
- [ ] `display_animation.ino` -- 帧动画（宠物表情切换）
- [ ] `display_weather.ino` -- 天气信息显示界面（加分项）
- [ ] 屏幕效果照片/视频

**评分标准：**
| 评价项 | 分值 | 说明 |
|--------|------|------|
| 显示驱动正确 | 30 分 | 屏幕点亮、正确显示内容 |
| 动画流畅性 | 30 分 | 帧率合理、无明显闪烁 |
| 界面设计 | 25 分 | 布局美观、信息层次清晰 |
| 代码结构 | 15 分 | 显示逻辑模块化 |

### 作业 2.3：摄像头流媒体 Demo | Camera Streaming Demo

**要求：**
- 实现 ESP32 摄像头的实时 MJPEG 流媒体传输
- Implement real-time MJPEG streaming from the ESP32 camera

**提交内容：**
- [ ] `camera_stream.ino` -- 摄像头流媒体服务代码
- [ ] 浏览器截图（显示实时画面）
- [ ] 流媒体性能数据（分辨率、帧率、延迟）

**评分标准：**
| 评价项 | 分值 | 说明 |
|--------|------|------|
| 流媒体功能正常 | 40 分 | 浏览器能看到实时画面 |
| 图像质量 | 25 分 | 分辨率合理、无明显偏色/条纹 |
| 性能指标 | 20 分 | 帧率 >= 10 FPS、延迟 < 500ms |
| 代码注释 | 15 分 | 关键参数有解释说明 |

---

## 阶段三作业 | Phase 3 Assignments

**主题：后端集成与 AI 对话 | Backend Integration & AI Chat**

### 作业 3.1：Python 后端集成代码 | Python Backend Integration

**要求：**
- 搭建 Python 后端服务器，实现与 ESP32 的通信
- Build a Python backend server that communicates with the ESP32

**提交内容：**
- [ ] `backend/` 目录完整代码
  - `main.py` 或 `app.py` -- 服务器入口
  - `requirements.txt` -- 依赖清单
  - `.env.example` -- 环境变量示例
  - `README.md` -- 运行说明
- [ ] API 接口文档（列出所有端点及用法）
- [ ] 后端运行截图（终端启动日志 + 浏览器访问结果）

**评分标准：**
| 评价项 | 分值 | 说明 |
|--------|------|------|
| 服务器功能完整 | 30 分 | 所有 API 端点正常工作 |
| 代码质量 | 25 分 | 模块化、注释充分、错误处理完善 |
| ESP32 通信 | 25 分 | ESP32 能正确调用后端 API |
| 文档完整性 | 20 分 | README 可复现、API 文档清晰 |

**Python 代码基本要求：**
- 使用 Python 3.10-3.12
- 使用 venv 虚拟环境
- 依赖通过 requirements.txt 管理
- API Key 通过 .env 文件配置（不硬编码）
- 关键函数有 docstring 注释

### 作业 3.2：AI 对话 Demo | AI Chat Demo

**要求：**
- 实现语音与 AI 大模型的对话功能
- Implement voice-based conversation with an AI large language model

**提交内容：**
- [ ] AI 对话功能演示视频（至少 2 分钟，展示完整对话流程）
- [ ] 对话日志文件（记录 API 请求和响应）
- [ ] 架构说明文档（语音 -> 文本 -> AI -> 文本 -> 语音 的数据流）

**评分标准：**
| 评价项 | 分值 | 说明 |
|--------|------|------|
| 语音识别准确 | 25 分 | ASR 能正确转写语音 |
| AI 回复质量 | 25 分 | LLM 回复连贯、有上下文理解 |
| 语音合成自然 | 25 分 | TTS 输出清晰自然 |
| 端到端延迟 | 15 分 | 从说话到听到回复 < 5 秒 |
| 多轮对话能力 | 10 分 | 能保持上下文连续对话 |

### 作业 3.3：最终项目报告 | Final Project Report

**要求：**
- 撰写完整的项目报告，总结整个开发过程
- Write a comprehensive project report summarizing the entire development process

**提交内容：**
- [ ] 项目报告（Markdown 或 PDF 格式，至少 2000 字）
  - 项目背景与目标
  - 系统架构设计（含架构图）
  - 硬件搭建过程
  - 软件设计与实现
  - AI 功能集成方案
  - 遇到的困难与解决方案
  - 学习反思与收获
  - 未来改进方向

**评分标准：**
| 评价项 | 分值 | 说明 |
|--------|------|------|
| 内容完整性 | 30 分 | 覆盖所有要求章节 |
| 技术深度 | 25 分 | 架构设计合理、技术选型有据 |
| 图表质量 | 20 分 | 架构图、流程图清晰专业 |
| 反思深度 | 15 分 | 真实记录困难与成长 |
| 排版规范 | 10 分 | 格式统一、图文并茂 |

---

## 提交格式要求 | Submission Format Requirements

### 文件组织

```
your-name-advanced/
├── phase1/
│   ├── hardware-docs/
│   │   ├── photos/              # 硬件搭建照片
│   │   ├── wiring-diagram.md    # 接线图文档
│   │   └── test-records.md      # 万用表测试记录
│   └── arduino/
│       ├── blink_basic/
│       ├── blink_button/
│       ├── blink_pwm/
│       └── blink_serial/
├── phase2/
│   ├── servo/
│   │   ├── servo_basic/
│   │   ├── servo_smooth/
│   │   └── servo_sequence/
│   ├── display/
│   │   ├── display_basic/
│   │   ├── display_animation/
│   │   └── display_weather/
│   └── camera/
│       └── camera_stream/
├── phase3/
│   ├── backend/
│   │   ├── app.py
│   │   ├── requirements.txt
│   │   ├── .env.example
│   │   └── README.md
│   ├── ai-demo/
│   │   └── demo-video.mp4
│   └── report/
│       └── final-report.md
├── weekly-reports/
│   ├── week1-checkin.md
│   └── week2-checkin.md
└── README.md
```

### 通用要求

1. **代码文件**：每个 Arduino sketch 放在独立文件夹中（Arduino IDE 要求），Python 代码遵循 PEP 8
2. **图片格式**：JPG/PNG，分辨率不低于 1280x720
3. **视频格式**：MP4，时长不超过 5 分钟
4. **文档格式**：Markdown（.md）优先，PDF 也可接受
5. **命名规范**：使用英文和连字符，如 `servo-sequence.ino`、`week1-checkin.md`
6. **语言要求**：文档和注释支持中文或英文，鼓励中英双语
7. **提交方式**：打包为 ZIP 文件或推送至 Git 仓库

---

## 学术诚信 | Academic Integrity

- **鼓励**：参考 Arduino 官方示例和库文档、使用搜索引擎、同学间讨论技术问题
- **允许**：使用 AI 辅助工具（如 ChatGPT）帮助理解概念和调试代码
- **允许**：参考开源项目的驱动代码，但需要理解和标注来源
- **要求**：提交的代码必须是自己理解和修改过的，不能直接克隆整个项目
- **要求**：如果使用了第三方库或参考了开源代码，必须在 README 中注明来源
- **禁止**：让他人代写代码或代焊 PCB

*最后更新：2026-05-27*
