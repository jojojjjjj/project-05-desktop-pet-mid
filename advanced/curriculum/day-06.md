# Day 06: 麦克风录音与语音处理 | Microphone Recording & Audio Processing

> **今日目标:**
> - 使用 XIAO Sense 板载麦克风采集音频
> - 理解音频采样率和位深
> - 实现音频录制并通过 WebSocket 发送
> - 基于音量的语音活动检测 (VAD)
>
> **产出:** 麦克风音频通过 WebSocket 传输到后端，能检测说话/静音

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间回顾 | 摄像头流媒体回顾、网络问题排查 |
| 09:15 - 10:30 | 知识讲解 | 数字音频基础、I2S 录音、音频格式、VAD |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | 麦克风录音 + 串口波形输出 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | WebSocket 音频传输 + VAD 检测 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 音频降噪、录音触控开关 |
| 16:30 - 17:00 | 总结分享 | 音频处理经验交流 |

---

## 上午: 数字音频与麦克风 | Morning: Digital Audio & Microphone

### 音频基础 | Audio Fundamentals

```
数字音频关键参数:
  采样率 (Sample Rate): 每秒采集多少个样本
    → 16kHz: 语音识别常用（够用且省带宽）
    → 44.1kHz: CD 音质
    → 48kHz: 专业音频

  位深 (Bit Depth): 每个样本用多少位表示
    → 16-bit: ±32767, 语音标准
    → 32-bit: 更高精度

  声道数 (Channels):
    → 单声道 (Mono): 语音识别只需单声道
    → 立体声 (Stereo): 音乐播放
```

### I2S 录音配置 | I2S Recording Config

XIAO Sense 板载麦克风通过 I2S 接口采集音频，需要配置第二个 I2S 通道（第一个用于播放）。

```cpp
#include <driver/i2s.h>

// 麦克风使用 I2S_NUM_1（避免与播放冲突）
#define MIC_I2S_NUM     I2S_NUM_1
#define MIC_SAMPLE_RATE 16000
#define MIC_BUFFER_LEN  1024

void setupMicrophone() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = MIC_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    // XIAO Sense 麦克风引脚
    i2s_pin_config_t pin_config = {
        .bck_io_num = 42,
        .ws_io_num = 41,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = 2  // DIN on GPIO2
    };

    i2s_driver_install(MIC_I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(MIC_I2S_NUM, &pin_config);
    Serial.println("Microphone ready!");
}
```

### 录音并计算音量 | Record & Calculate Volume

```cpp
int16_t audio_buffer[MIC_BUFFER_LEN];

float readAudioLevel() {
    size_t bytes_read;
    i2s_read(MIC_I2S_NUM, audio_buffer, MIC_BUFFER_LEN * 2,
             &bytes_read, portMAX_DELAY);

    // 计算 RMS (Root Mean Square) 音量
    float sum = 0;
    int samples = bytes_read / 2;
    for (int i = 0; i < samples; i++) {
        sum += (float)audio_buffer[i] * audio_buffer[i];
    }
    return sqrt(sum / samples);
}
```

### 语音活动检测 (VAD) | Voice Activity Detection

```cpp
#define VAD_THRESHOLD 500  // 音量阈值（需根据实际校准）
#define SILENCE_DURATION 1500  // 静音持续时间判定结束（ms）

bool isSpeaking = false;
unsigned long lastVoiceTime = 0;

void checkVAD() {
    float level = readAudioLevel();

    if (level > VAD_THRESHOLD) {
        if (!isSpeaking) {
            Serial.println("Voice started!");
            isSpeaking = true;
        }
        lastVoiceTime = millis();
    } else if (isSpeaking && (millis() - lastVoiceTime > SILENCE_DURATION)) {
        Serial.println("Voice ended.");
        isSpeaking = false;
    }
}
```

---

## 下午: WebSocket 音频传输 | Afternoon: Audio WebSocket

### 音频数据发送 | Send Audio via WebSocket

```cpp
void streamAudio() {
    size_t bytes_read;
    i2s_read(MIC_I2S_NUM, audio_buffer, MIC_BUFFER_LEN * 2,
             &bytes_read, portMAX_DELAY);

    if (audioWS.available()) {
        audioWS.sendBinary((const char*)audio_buffer, bytes_read);
    }
}
```

### 双通道架构 | Dual-Channel Architecture

```
ESP32 与后端的 WebSocket 通道:

/ws/camera  →  视频流（JPEG 帧，~20fps）
/ws/audio   →  音频流（PCM 16bit，16kHz）
/ws/control ←→ 控制命令（表情、舵机、文本）
```

---

## 拓展练习 | Extension Exercises

### 练习1: 录音回放
用麦克风录制 2 秒音频，然后通过扬声器播放出来（注意 I2S 通道切换）。

### 练习2: 拍手检测
利用 VAD 检测拍手声，检测到时让猫做"惊讶"表情。

---

## 今日总结 | Day Summary

| 知识点 | 掌握程度 |
|--------|----------|
| 数字音频参数 | 理解采样率、位深、声道 |
| I2S 录音 | 能配置并采集麦克风音频 |
| 音量检测 (RMS) | 能计算音频能量 |
| VAD | 能检测说话/静音状态 |
| 音频 WebSocket | 能将音频流发送到后端 |

*最后更新：2026-05-27*
