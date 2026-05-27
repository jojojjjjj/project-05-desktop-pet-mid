# Day 04: 硬件组装与音频系统 | Hardware Assembly & Audio System

> **今日目标:**
> - 理解I2S音频接口原理
> - 完成MAX98357A音频模块的接线与测试
> - 完成所有硬件的面包板接线
> - 理解共地（Common Ground）和供电设计的重要性
>
> **产出:** 所有硬件在面包板上正确连接，扬声器能播放测试音频

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间回顾 | 回顾昨日作业，解答疑问 |
| 09:15 - 10:30 | 知识讲解 | I2S协议、MAX98357A原理、供电设计 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | MAX98357A接线与音频播放测试 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | 面包板完整接线与供电检查 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 播放旋律、音量控制 |
| 16:30 - 17:00 | 总结分享 | 今日收获总结 |

## 上午: I2S音频系统 | Morning: I2S Audio System

### 1. I2S协议 | I2S Protocol

I2S（Inter-IC Sound）是专为数字音频设计的串行接口，用三根线传输音频数据：

I2S (Inter-IC Sound) is a serial interface designed specifically for digital audio, using three wires:

```
I2S 三线制 Three-wire interface:
  BCLK (Bit Clock)  → 位时钟，同步数据传输 Bit clock
  LRC  (Left/Right) → 左右声道选择 Channel select
  DIN  (Data In)    → 音频串行数据 Audio serial data

数据格式 Data format:
  16-bit, 16kHz, 单声道 Mono
  每秒传输 16000 x 16bit = 256kbps
```

### 2. MAX98357A接线 | MAX98357A Wiring

| MAX98357A引脚 | XIAO引脚 | GPIO | 说明 Description |
|--------------|---------|------|----------------|
| BCLK | D3 | GPIO4 | 位时钟 Bit clock |
| LRC | D2 | GPIO3 | 左右声道 Left/Right |
| DIN | D9 | GPIO8 | 音频数据 Audio data |
| VCC | 5V | - | 供电（必须是5V Must be 5V） |
| GND | GND | - | 公共地 Common ground |
| GAIN | -- | - | 悬空=9dB, 接GND=12dB |
| SD | -- | - | 悬空或VCC=正常工作 |

### 3. 音频播放测试 | Audio Playback Test

```cpp
// Day 04 - I2S 音频初始化与播放
#include <driver/i2s.h>

#define I2S_BCLK  4   // D3 GPIO4
#define I2S_LRC   3   // D2 GPIO3
#define I2S_DIN   8   // D9 GPIO8

void i2sInit() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 16000,                        // 16kHz采样率
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // 16位深度
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  // 单声道
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  esp_err_t result = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  if (result != ESP_OK) {
    Serial.printf("I2S driver install failed: %d\n", result);
    return;
  }

  i2s_set_pin(I2S_NUM_0, &pin_config);
  Serial.println("I2S audio initialized successfully!");
}

// 播放指定频率的音调 Play a tone at given frequency
void playTone(int freq, int duration_ms) {
  int sample_rate = 16000;
  int samples = sample_rate * duration_ms / 1000;
  int16_t *buf = (int16_t*)malloc(samples * sizeof(int16_t));

  if (buf == NULL) {
    Serial.println("Failed to allocate audio buffer!");
    return;
  }

  for (int i = 0; i < samples; i++) {
    float t = (float)i / sample_rate;
    buf[i] = (int16_t)(sin(2 * PI * freq * t) * 16000);  // 50%振幅
  }

  size_t bytes_written;
  i2s_write(I2S_NUM_0, buf, samples * 2, &bytes_written, portMAX_DELAY);
  free(buf);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== I2S Audio Test ===");

  i2sInit();

  // 播放测试音 Play test tones
  Serial.println("Playing test tone: 440Hz...");
  playTone(440, 500);  // A4 音 0.5秒
  delay(200);
  playTone(880, 500);  // A5 音 0.5秒
}

void loop() {}
```

### 4. 播放WAV音频 | Playing WAV Audio

```cpp
// Day 04 - 播放简单音效 Play simple sound effects

// 生成并播放"喵"声 Generate and play a "meow" sound
void playMeow() {
  int sample_rate = 16000;
  int duration = 400;  // 400ms
  int samples = sample_rate * duration / 1000;
  int16_t *buf = (int16_t*)malloc(samples * 2);

  // "喵"声 = 从800Hz滑降到400Hz
  for (int i = 0; i < samples; i++) {
    float t = (float)i / sample_rate;
    float progress = (float)i / samples;

    // 频率从800Hz滑到400Hz Frequency sweep 800->400Hz
    float freq = 800 - 400 * progress;

    // 振幅包络 Amplitude envelope
    float envelope = 1.0;
    if (progress < 0.1) envelope = progress / 0.1;          // 淡入 Fade in
    else if (progress > 0.7) envelope = (1.0 - progress) / 0.3;  // 淡出 Fade out

    buf[i] = (int16_t)(sin(2 * PI * freq * t) * 16000 * envelope);
  }

  size_t bytes_written;
  i2s_write(I2S_NUM_0, buf, samples * 2, &bytes_written, portMAX_DELAY);
  free(buf);
}

// 播放"嘟嘟"提示音 Play beep sound
void playBeep() {
  playTone(1000, 100);  // 1kHz, 100ms
  delay(100);
  playTone(1000, 100);
}
```

## 下午: 完整面包板接线 | Afternoon: Full Breadboard Wiring

### 5. 供电设计 | Power Supply Design

**关键原则：舵机必须独立供电！**

**Critical rule: Servos MUST have independent power supply!**

```
供电架构 Power Architecture:

USB 5V ──→ XIAO ESP32S3 (逻辑供电 Logic power)
              │
              ├── 3.3V ──→ ST7789 (VCC)
              │            PCA9685 VCC (逻辑 Logic)
              │
外部 5V/6V ──→ PCA9685 V+ (舵机动力 Servo power)
              │
              └── GND ←──→ XIAO GND (必须共地! MUST share ground!)
```

**为什么需要共地？** 所有设备必须参考相同的0V电压，否则信号无法正确识别。
**Why common ground?** All devices must reference the same 0V, or signals won't be recognized correctly.

### 6. 完整接线检查清单 | Complete Wiring Checklist

| 模块 Module | 信号线 Signal | 电源线 Power | 检查 |
|------------|-------------|-------------|------|
| ST7789屏幕 | CS=D4, DC=D0, SCK=D10, MOSI=D8, RST=D1 | 3.3V, GND | [ ] |
| PCA9685驱动 | SDA=D6, SCL=D7 | 3.3V(逻辑), 外部5V(舵机), GND | [ ] |
| MAX98357A | BCLK=D3, LRC=D2, DIN=D9 | 5V, GND | [ ] |
| STS3032总线 | TX=D8(GPIO43), RX=D9(GPIO44) | 外部电源, GND | [ ] |
| SG90舵机x4 | CH12(嘴), CH13(尾), CH14(左耳), CH15(右耳) | PCA9685 V+ | [ ] |
| 摄像头 Camera | 板载 Onboard | 板载 Onboard | [ ] |
| 麦克风 Mic | 板载 Onboard | 板载 Onboard | [ ] |

> **注意：** STS3032使用UART1（Serial1），引脚与SPI的MOSI(D8)可能冲突。实际接线时需确认引脚分配，或使用软件串口。Check for pin conflicts between STS3032 UART and SPI MOSI.

### 7. 验证步骤 | Verification Steps

```cpp
// Day 04 - 系统自检程序 System self-test
// 逐模块检测硬件连接 Check hardware connections module by module

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== System Self-Test ===\n");

  // 1. I2C扫描 - 检测PCA9685
  Serial.println("[1/3] I2C Scan...");
  Wire.begin(6, 7);
  bool pca_found = false;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  Found I2C device at 0x%02X\n", addr);
      if (addr == 0x40) pca_found = true;
    }
  }
  Serial.println(pca_found ? "  PCA9685: OK" : "  PCA9685: NOT FOUND!");

  // 2. SPI屏幕测试
  Serial.println("[2/3] ST7789 Test...");
  tft.init(172, 320);
  tft.fillScreen(ST77XX_RED);
  delay(500);
  tft.fillScreen(ST77XX_GREEN);
  delay(500);
  tft.fillScreen(ST77XX_BLUE);
  delay(500);
  tft.fillScreen(ST77XX_BLACK);
  Serial.println("  ST7789: OK");

  // 3. I2S音频测试
  Serial.println("[3/3] I2S Audio Test...");
  i2sInit();
  playTone(440, 200);
  Serial.println("  Audio: OK");

  Serial.println("\n=== Self-Test Complete ===");
}

void loop() {}
```

## 拓展练习 | Extension Exercises

### 练习1: 播放旋律 | Play a Melody

用 `playTone()` 播放"小星星"旋律：

```cpp
// 小星星 Twinkle Twinkle Little Star
// 音符频率 Note frequencies: C4=262 D4=294 E4=330 F4=349 G4=392 A4=440 B4=494

void playTwinkle() {
  int melody[] = {
    262, 262, 392, 392, 440, 440, 392,  // C C G G A A G
    349, 349, 330, 330, 294, 294, 262   // F F E E D D C
  };
  int durations[] = {
    400, 400, 400, 400, 400, 400, 800,
    400, 400, 400, 400, 400, 400, 800
  };

  for (int i = 0; i < 14; i++) {
    playTone(melody[i], durations[i]);
    delay(50);  // 音符间短暂停顿 Short gap between notes
  }
}
```

### 练习2: 音量控制 | Volume Control

```cpp
// 通过改变振幅控制音量 Control volume by changing amplitude
void playToneWithVolume(int freq, int duration_ms, float volume) {
  int sample_rate = 16000;
  int samples = sample_rate * duration_ms / 1000;
  int16_t *buf = (int16_t*)malloc(samples * 2);

  int16_t amplitude = (int16_t)(32767 * volume);  // volume: 0.0 ~ 1.0

  for (int i = 0; i < samples; i++) {
    float t = (float)i / sample_rate;
    buf[i] = (int16_t)(sin(2 * PI * freq * t) * amplitude);
  }

  size_t bytes_written;
  i2s_write(I2S_NUM_0, buf, samples * 2, &bytes_written, portMAX_DELAY);
  free(buf);
}
```

### 练习3: 组合测试 | Combined Test
结合屏幕和音频：播放音效时同时在屏幕上显示对应表情。

## 今日总结 | Day Summary

| 知识点 | 掌握程度 |
|--------|----------|
| I2S音频接口 | 理解BCLK/LRC/DIN三线制与数字音频传输 |
| MAX98357A驱动 | 能初始化I2S并通过功放播放音频 |
| 音频生成 | 能用代码生成不同频率的音调和音效 |
| 供电设计 | 理解舵机独立供电和共地的重要性 |
| 面包板接线 | 完成全部模块接线与验证 |
| 调试方法 | 逐模块验证、万用表检查、自检程序 |

- **明日预告**: 摄像头流媒体与WebSocket通信
