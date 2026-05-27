# Day 12: 项目优化与最终展示 | Project Polish & Final Presentation

> **今日目标:**
> - 修复已知 Bug，提升系统稳定性
> - 优化性能（延迟、流畅度）
> - 设计自定义表情和创意动作
> - 准备并完成最终项目展示
>
> **产出:** 一个稳定、完整、有创意的 AI 桌面猫项目展示

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间回顾 | 全链路集成经验回顾 |
| 09:15 - 10:30 | 项目优化 | Bug 修复、稳定性提升、性能优化 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 创意定制 | 自定义表情、创意动作、个性化设定 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 展示准备 | PPT/网站准备、演示流程排练 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 17:00 | 最终展示 | 每人 10 分钟演示 + Q&A |

---

## 上午: 项目优化 | Morning: Polish & Optimize

### 常见 Bug 清单 | Common Bug Checklist

| 问题 | 原因 | 修复方案 |
|------|------|----------|
| WiFi 断连后不重连 | 未处理断连事件 | 添加 WiFi 事件回调 + 重连逻辑 |
| WebSocket 超时 | 长时间无数据 | 心跳包 + 超时重连 |
| 音频播放卡顿 | I2S DMA 缓冲区不够 | 增大 dma_buf_count |
| 表情帧跳帧 | JPEG 解码太慢 | 预解码或降低分辨率 |
| 舵机抖动 | PWM 频率不准 | 校准 PCA9685 频率为 50Hz |
| AI 回复慢 | 网络延迟 | 使用流式输出 + 减短 system prompt |
| 内存溢出 | 未释放帧缓冲 | 确保 esp_camera_fb_return() |

### 性能优化建议 | Performance Tips

```cpp
// 1. 摄像头优化: 降低分辨率
config.frame_size = FRAMESIZE_QQVGA;  // 160x120，更流畅

// 2. 音频缓冲优化
config.dma_buf_count = 16;  // 更多缓冲区
config.dma_buf_len = 2048;  // 每个更大

// 3. WebSocket 心跳
void sendHeartbeat() {
    static unsigned long lastHB = 0;
    if (millis() - lastHB > 10000) {
        ws.send("{\"type\":\"ping\"}");
        lastHB = millis();
    }
}
```

### WiFi 断线重连 | WiFi Reconnect

```cpp
void checkWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi lost, reconnecting...");
        WiFi.reconnect();
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            attempts++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("WiFi reconnected!");
            connectWS();  // 重新连接 WebSocket
        }
    }
}
```

---

## 下午: 创意定制与展示 | Afternoon: Creative Extensions & Presentation

### 创意方向参考 | Creative Ideas

| 方向 | 说明 | 难度 |
|------|------|------|
| 自定义表情包 | 用图片编辑软件制作独特的猫咪表情帧 | ⭐ |
| 舞蹈动作 | 编排一段有节奏感的舵机动作序列 | ⭐⭐ |
| 多角色切换 | 不同性格的猫咪角色（高冷/活泼/萌） | ⭐⭐ |
| 定时问候 | 定时器触发猫咪主动打招呼 | ⭐⭐ |
| 天气播报 | 调用天气 API，猫咪播报今日天气 | ⭐⭐⭐ |
| 闹钟功能 | 设定时间，猫咪叫醒 | ⭐⭐ |
| 多猫互动 | 两只猫通过 WebSocket 对话 | ⭐⭐⭐⭐ |

### 展示要求 | Presentation Requirements

**10 分钟展示（5 分钟演示 + 5 分钟 Q&A）**

必须展示的功能：
1. **语音对话**: 说一句话，AI 回复 + 语音播放
2. **表情同步**: AI 回复时表情变化
3. **行走动作**: 四足行走或坐下
4. **Web 控制面板**: 浏览器中查看摄像头和控制

展示加分项：
- 系统架构图
- 自定义创意功能
- 技术难点分享
- 学习心得

### 展示流程建议 | Presentation Flow

```
1. 开场 (30秒)
   "大家好，这是我的 AI 桌面猫 [名字]"
   展示成品外观

2. 功能演示 (3分钟)
   - 打招呼触发 AI 对话
   - 猫咪走路 + 摇尾巴
   - 表情变化
   - Web 控制面板展示

3. 技术讲解 (1.5分钟)
   - 架构图：ESP32 ↔ Python ↔ Qwen
   - 关键技术点
   - 遇到的最大挑战

4. 总结 (30秒)
   - 学到了什么
   - 未来可以怎么改进
```

---

## 课程总结 | Course Summary

### 12 天技术栈回顾 | 12-Day Tech Stack Review

| 天数 | 技术 | 核心概念 |
|------|------|----------|
| Day 1-2 | Arduino IDE + I2C + PWM | 开发环境、通信协议、舵机控制 |
| Day 3-4 | SPI + I2S + 面包板接线 | 屏幕驱动、音频播放、硬件组装 |
| Day 5-6 | 摄像头 + WebSocket + 麦克风 | 网络通信、音视频采集、流媒体 |
| Day 7-8 | STS3032 + PCA9685 + LittleFS | 四足步态、多舵机协调、文件系统 |
| Day 9-10 | Python + ASR + Qwen 3.5 Omni | 后端开发、语音识别、多模态 AI |
| Day 11-12 | 全链路集成 + 优化展示 | 系统集成、性能调优、项目展示 |

### 学到的核心能力 | Core Skills Acquired

1. **嵌入式开发**: Arduino IDE + ESP32 + 多种通信协议
2. **硬件集成**: 舵机、屏幕、摄像头、音频模块的系统集成
3. **后端开发**: Python + WebSocket 服务器
4. **AI 集成**: 多模态大模型 API 调用、ASR、TTS
5. **系统思维**: 从硬件到软件到 AI 的全链路理解
6. **项目展示**: 技术表达、演示设计、文档撰写

*最后更新：2026-05-27*
