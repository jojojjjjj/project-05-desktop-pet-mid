# Day 08: 音频系统与蓝牙音箱 | Audio System & Bluetooth Speaker

> **今日目标:**
> - I2S音频驱动开发
> - ES8311编解码器配置
> - 录音和音频播放
> - 蓝牙A2DP音箱功能
>
> **产出:** SparkBot能够录音、播放音频、作为蓝牙音箱使用，并支持TTS语音输出

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间活动 | 摄像头功能展示与问题解答 |
| 09:15 - 10:30 | 知识讲解 | I2S协议、ES8311编解码器、音频信号链 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | 配置I2S、初始化ES8311、播放测试音 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | 录音功能、蓝牙A2DP音箱 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 小度APP配网、TTS文字转语音 |
| 16:30 - 17:00 | 总结分享 | 蓝牙音箱功能演示、TTS播报展示 |

---

## 上午: I2S与ES8311音频系统 | Morning: I2S & ES8311 Audio System

### 为什么要学这个? | Why Learn This?

"能听会说"是AI桌面机器人的核心能力。音频系统让SparkBot能够：听到你的唤醒词、录制语音指令、播放AI语音回复、甚至作为蓝牙音箱播放手机音乐。I2S是数字音频传输的标准接口，ES8311则是将数字音频和模拟音频互相转换的编解码芯片(Codec)。

"Hearing and speaking" is the core capability of any AI desktop robot. The audio system enables SparkBot to: hear wake words, record voice commands, play AI speech responses, and even act as a Bluetooth speaker. I2S is the standard digital audio interface, and ES8311 converts between digital and analog audio.

**SparkBot音频信号链:**
```
录音路径 (MEMS麦克风 → ESP32-S3):
  MEMS麦克风 ──模拟──> ES8311 ADC ──I2S数字──> ESP32-S3 I2S RX

播放路径 (ESP32-S3 → 喇叭):
  ESP32-S3 I2S TX ──I2S数字──> ES8311 DAC ──模拟──> NS4150B功放 ──> 喇叭
```

### 任务8.1: I2S与ES8311初始化 (50分钟)

I2S (Inter-IC Sound) 是标准的数字音频传输总线，3根信号线：位时钟(BCLK)、声道选择(WS/LRCK)、数据(DIN/DOUT)，外加可选的主时钟(MCLK)。

```c
#include "driver/i2s_std.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "AUDIO";

// I2S 引脚定义 (根据SparkBot原理图)
#define I2S_MCLK_IO     GPIO_NUM_5     // 主时钟 (256×Fs)
#define I2S_BCLK_IO     GPIO_NUM_6     // 位时钟
#define I2S_WS_IO       GPIO_NUM_7     // 左右声道选择 (LRCK)
#define I2S_DOUT_IO     GPIO_NUM_8     // 数据输出 (ESP→ES8311, 播放)
#define I2S_DIN_IO      GPIO_NUM_9     // 数据输入 (ES8311→ESP, 录音)

// ES8311 I2C地址
#define ES8311_I2C_ADDR     0x18
#define I2C_MASTER_NUM      I2C_NUM_0

static i2s_chan_handle_t tx_chan;       // 发送通道(播放)
static i2s_chan_handle_t rx_chan;       // 接收通道(录音)

// ---- I2S 初始化 (ESP-IDF v5.x新版API) ----
static esp_err_t i2s_audio_init(void)
{
    ESP_LOGI(TAG, "初始化I2S音频接口...");

    // TX通道 (播放)
    i2s_chan_config_t tx_cfg = {
        .id = I2S_NUM_0,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 8,
        .dma_frame_num = 256,
        .auto_clear = true,
    };
    ESP_ERROR_CHECK(i2s_new_channel(&tx_cfg, &tx_chan, NULL));

    i2s_std_config_t tx_std = {
        .clk_cfg = {
            .sample_rate_hz = 44100,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .mclk_multiple = I2S_MCLK_MULTIPLE_256,
        },
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
            .slot_mode = I2S_SLOT_MODE_STEREO,
            .slot_mask = I2S_STD_SLOT_BOTH,
            .ws_width = I2S_DATA_BIT_WIDTH_16BIT,
            .ws_pol = false,
            .bit_shift = true,          // I2S Philips标准
            .msb_right = false,
        },
        .gpio_cfg = {
            .mclk = I2S_MCLK_IO,
            .bclk = I2S_BCLK_IO,
            .ws   = I2S_WS_IO,
            .dout = I2S_DOUT_IO,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false, .bclk_inv = false, .ws_inv = false
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &tx_std));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));

    // RX通道 (录音)
    i2s_chan_config_t rx_cfg = {
        .id = I2S_NUM_0,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 8,
        .dma_frame_num = 256,
        .auto_clear = true,
    };
    ESP_ERROR_CHECK(i2s_new_channel(&rx_cfg, NULL, &rx_chan));

    i2s_std_config_t rx_std = tx_std;
    rx_std.gpio_cfg.dout = I2S_GPIO_UNUSED;
    rx_std.gpio_cfg.din  = I2S_DIN_IO;
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &rx_std));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));

    ESP_LOGI(TAG, "I2S初始化完成 (TX+RX, 44.1kHz 16bit Stereo)");
    return ESP_OK;
}

// ---- ES8311 Codec初始化 (通过I2C寄存器配置) ----
static esp_err_t es8311_write_reg(uint8_t reg, uint8_t val)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ES8311_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, val, true);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd,
                                         100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t es8311_init(void)
{
    ESP_LOGI(TAG, "初始化ES8311音频编解码器...");

    // 复位芯片
    es8311_write_reg(0x00, 0x1F);
    vTaskDelay(pdMS_TO_TICKS(10));
    es8311_write_reg(0x00, 0x00);
    vTaskDelay(pdMS_TO_TICKS(10));

    // 时钟配置 (MCLK=256×Fs, BCLK=64×Fs)
    es8311_write_reg(0x01, 0x0F);   // 主时钟: 使能MCLK
    es8311_write_reg(0x02, 0x0C);   // 采样率/时钟分频

    // ADC配置 (录音通路)
    es8311_write_reg(0x09, 0x88);   // ADC上电 + 增益0dB
    es8311_write_reg(0x0B, 0x08);   // ADC格式: I2S, 16bit

    // DAC配置 (播放通路)
    es8311_write_reg(0x10, 0xC8);   // DAC上电, 不静音
    es8311_write_reg(0x11, 0x00);   // DAC音量: 0dB
    es8311_write_reg(0x12, 0x08);   // DAC格式: I2S, 16bit

    // 模拟通路 — 麦克风偏置和增益
    es8311_write_reg(0x18, 0x44);   // 麦克风偏置: 2.8V
    es8311_write_reg(0x20, 0x30);   // PGA增益: 18dB

    // 模拟通路 — 喇叭输出
    es8311_write_reg(0x2D, 0x20);   // 扬声器输出使能

    ESP_LOGI(TAG, "ES8311初始化完成!");
    return ESP_OK;
}

// ---- 播放1kHz正弦波测试音 ----
static void play_test_tone(void)
{
    #define TONE_MS     500
    #define SAMPLE_RATE 44100
    #define FREQ        1000

    int samples = SAMPLE_RATE * TONE_MS / 1000;
    int16_t *buf = malloc(samples * 2 * sizeof(int16_t));  // 立体声

    for (int i = 0; i < samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        int16_t s = (int16_t)(16384.0f * sinf(2.0f * M_PI * FREQ * t));
        buf[i * 2]     = s;  // 左声道
        buf[i * 2 + 1] = s;  // 右声道
    }

    ESP_LOGI(TAG, "播放1kHz测试音 (%d ms)...", TONE_MS);
    size_t written;
    i2s_channel_write(tx_chan, buf, samples * 4, &written, portMAX_DELAY);
    ESP_LOGI(TAG, "测试音播放完毕! 写入 %d 字节", written);
    free(buf);
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== SparkBot 音频系统演示 ===");

    // 初始化I2C (用于ES8311配置)
    i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_42,
        .scl_io_num = GPIO_NUM_41,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    i2c_param_config(I2C_MASTER_NUM, &i2c_cfg);
    i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, 0, 0, 0);

    // 初始化音频系统
    i2s_audio_init();
    es8311_init();

    // 播放测试音
    play_test_tone();
    ESP_LOGI(TAG, "音频系统就绪!");
}
```

**I2S关键参数:**

| 参数 | 语音场景 | 音乐场景 | 说明 |
|------|---------|---------|------|
| 采样率 | 16kHz | 44.1kHz | 语音可降低采样率省资源 |
| 位深 | 16bit | 16bit | CD品质标准 |
| 声道 | Mono | Stereo | 语音单声道即可 |
| MCLK倍数 | 256x | 256x | MCLK = 倍数 × Fs |

**预期结果:**
- 喇叭播放1kHz测试音(0.5秒"嘟"声)
- 无噪音、无杂音
- 串口显示I2S和ES8311初始化成功

**常见问题:**
- **I2C地址不响应**：检查ES8311的ADDR引脚电平(地址0x18或0x19)
- **没有声音**：检查功放NS4150B的SHDN引脚是否拉高、MCLK频率是否正确
- **声音是噪音**：I2S时序不对，检查WS极性和数据格式(I2S标准 vs 左对齐)
- **录音无声**：检查MEMS麦克风偏置电压(0x18寄存器)和PGA增益(0x20寄存器)

### 任务8.2: 录音功能 (30分钟)

```c
#define RECORD_BUF_SIZE     4096

// 录音 → 保存WAV文件
static esp_err_t record_and_save_wav(const char *filepath, int duration_sec)
{
    ESP_LOGI(TAG, "开始录音 %d 秒...", duration_sec);

    FILE *f = fopen(filepath, "wb");
    if (!f) { ESP_LOGE(TAG, "无法创建文件"); return ESP_FAIL; }

    // 写入WAV文件头 (44字节)
    int sample_rate = 44100, bits = 16, channels = 1;
    int data_size = sample_rate * channels * (bits / 8) * duration_sec;

    // RIFF头
    uint8_t wav_header[44] = {0};
    memcpy(wav_header,     "RIFF", 4);
    memcpy(wav_header + 8,  "WAVE", 4);
    memcpy(wav_header + 12, "fmt ", 4);
    int32_t *p = (int32_t *)(wav_header + 4);  *p = data_size + 36;
    p = (int32_t *)(wav_header + 16);  *p = 16;         // fmt size
    int16_t *sp = (int16_t *)(wav_header + 20); *sp = 1; // PCM
    sp = (int16_t *)(wav_header + 22); *sp = channels;
    p = (int32_t *)(wav_header + 24);  *p = sample_rate;
    p = (int32_t *)(wav_header + 28);  *p = sample_rate * channels * bits / 8;
    sp = (int16_t *)(wav_header + 32); *sp = channels * bits / 8;
    sp = (int16_t *)(wav_header + 34); *sp = bits;
    memcpy(wav_header + 36, "data", 4);
    p = (int32_t *)(wav_header + 40);  *p = data_size;
    fwrite(wav_header, 1, 44, f);

    // 录音循环
    int16_t *buf = malloc(RECORD_BUF_SIZE);
    int total = 0, target = data_size;
    TickType_t start = xTaskGetTickCount();

    while (total < target) {
        size_t bytes_read = 0;
        i2s_channel_read(rx_chan, buf, RECORD_BUF_SIZE,
                         &bytes_read, pdMS_TO_TICKS(100));
        if (bytes_read > 0) {
            fwrite(buf, 1, bytes_read, f);
            total += bytes_read;
        }
        if ((xTaskGetTickCount() - start) > pdMS_TO_TICKS(duration_sec * 2)) break;
    }

    free(buf); fclose(f);
    ESP_LOGI(TAG, "录音完成! %d bytes → %s", total, filepath);
    return ESP_OK;
}

// 录音回放
static void playback_wav(const char *filepath)
{
    FILE *f = fopen(filepath, "rb");
    if (!f) { ESP_LOGE(TAG, "文件不存在: %s", filepath); return; }

    fseek(f, 44, SEEK_SET);  // 跳过WAV头
    ESP_LOGI(TAG, "播放: %s", filepath);

    int16_t buf[1024];
    size_t read;
    while ((read = fread(buf, 1, sizeof(buf), f)) > 0) {
        size_t written;
        i2s_channel_write(tx_chan, buf, read, &written, portMAX_DELAY);
    }
    fclose(f);
    ESP_LOGI(TAG, "播放完毕!");
}
```

---

## 下午: 蓝牙音箱与TTS | Afternoon: Bluetooth Speaker & TTS

### 任务8.3: 蓝牙A2DP音箱 (40分钟)

A2DP (Advanced Audio Distribution Profile) 是蓝牙音频传输标准协议。SparkBot作为A2DP Sink (接收端)，手机作为Source，将手机音乐通过蓝牙传输到SparkBot播放。

```c
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_a2dp_api.h"
#include "esp_bt_device.h"

static void bluetooth_a2dp_init(void)
{
    ESP_LOGI(TAG, "初始化蓝牙A2DP音箱...");

    // 1. 释放BLE内存 (如不需要BLE)
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    // 2. 初始化蓝牙控制器 (经典蓝牙)
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));

    // 3. 初始化Bluedroid协议栈
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    // 4. 设置设备名称
    esp_bt_dev_set_device_name("ESP-SparkBot");

    // 5. 注册A2DP回调 → 初始化Sink → 设为可发现
    // esp_a2d_register_callback(&a2dp_cb);
    // esp_a2d_sink_init();
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

    ESP_LOGI(TAG, "蓝牙音箱就绪! 请在手机上搜索 'ESP-SparkBot'");
}

// 通过小度APP配网 (BluFi)
// 流程: SparkBot开机无WiFi → 手机小度APP通过蓝牙搜索 →
//       输入WiFi密码 → 加密传输 → SparkBot连接WiFi
// 完整示例: esp-idf/examples/bluetooth/bluedroid/classic_bt/a2dp_sink/
```

**蓝牙A2DP数据流:**
```
手机Source ──SBC/AAC──> SparkBot Sink ──SBC解码──> PCM
                                                  I2S TX → ES8311 → 喇叭
```

**预期结果:**
- 手机蓝牙搜索到 "ESP-SparkBot"
- 连接后手机音乐从SparkBot喇叭输出
- 串口打印连接状态和音频参数

### 任务8.4: TTS文字转语音 (30分钟)

TTS (Text-to-Speech) 将文字转为自然语音，是AI对话的"最后一公里"。

```c
// ---- TTS方案选择 ----
// 方案A: 云端TTS (需要WiFi, Day 9-10实现)
//   - 百度/讯飞TTS API: HTTP POST文字 → 返回MP3 → I2S播放
//   - 优点: 音质自然 | 缺点: 需网络, 延迟200-500ms
//
// 方案B: 本地TTS (离线)
//   - ESP-SR框架内置TTS引擎
//   - 优点: 低延迟, 无需网络 | 缺点: 音质一般

// 云端TTS调用示例 (Day 9-10网络课程完善):
static void cloud_tts_speak(const char *text)
{
    ESP_LOGI(TAG, "云端TTS播报: \"%s\"", text);
    // 1. HTTP POST: text → TTS API
    // 2. 接收MP3/WAV响应
    // 3. 解码并I2S播放
    // 4. 完整实现见Day 9-10
}

// 本地预录WAV播放 (在TTS就绪前的简化替代):
static const char *prompts[] = {
    "/sdcard/audio/ready.wav",    // "我在听"
    "/sdcard/audio/hello.wav",    // "你好"
    "/sdcard/audio/bye.wav",      // "再见"
};

static void speak_prompt(int id)
{
    if (id >= 0 && id < 3) {
        ESP_LOGI(TAG, "播放: %d", id);
        playback_wav(prompts[id]);
    }
}
```

**TTS方案对比:**

| 方案 | 音质 | 延迟 | 需要WiFi | 存储 | 适合场景 |
|------|------|------|---------|------|---------|
| 云端TTS | 优秀 | 200-500ms | 是 | 无 | AI对话回复 |
| 本地TTS | 一般 | <50ms | 否 | 2-5MB | 系统提示 |
| 预录WAV | 优秀 | <10ms | 否 | 每句10-50KB | 固定对话 |
| 蓝牙A2DP | 优秀 | 30-50ms | 否 | 无 | 音乐播放 |

---

## 今日作业 | Homework

### 必做题
1. 完成I2S初始化和ES8311配置，播放1kHz测试音
2. 实现录音功能，保存为WAV文件并回放验证
3. 在串口打印录音过程中的音频RMS电平值

### 挑战题
1. **录音并播放**: 按BOOT键开始录音5秒，松开后自动回放(建议用状态机管理录音/播放状态)
2. **蓝牙连接测试**: 将A2DP Sink示例集成到项目，用手机连接播放音乐并确认喇叭输出
3. **TTS播报天气**: 调用云端TTS API或预录WAV，用语音播报当前的温度和天气

### 思考题
1. I2S和I2C有什么关系和本质区别？（提示：名字相似但用途完全不同）
2. 为什么需要ES8311这样的Codec芯片？ESP32-S3内置的ADC/DAC不能直接驱动麦克风和喇叭吗？
3. A2DP传输的是原始PCM数据还是压缩音频？为什么蓝牙需要SBC或AAC编码？

---

## 明日预告 | Tomorrow's Preview

明天我们将让SparkBot连接WiFi，获取网络时间和天气数据！你将学习WiFi Station模式、HTTP客户端、JSON解析和NTP时间同步。SparkBot将从"离线玩具"华丽升级为"联网智能设备"！

Tomorrow we connect SparkBot to WiFi for network time and weather data! Learn WiFi Station mode, HTTP client, JSON parsing, and NTP time sync. SparkBot upgrades from "offline toy" to "connected smart device"!

---

## 参考资源 | References

- [ESP-IDF I2S 驱动指南](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/peripherals/i2s.html)
- [ES8311 数据手册](https://www.everest-semi.com/pdf/ES8311%20PB.pdf)
- [ESP-IDF 蓝牙A2DP Sink示例](https://github.com/espressif/esp-idf/tree/master/examples/bluetooth/bluedroid/classic_bt/a2dp_sink)
- [ESP-BluFi 配网指南](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/bluetooth/esp_blufi.html)
- [ESP-SR 语音识别框架 (GitHub)](https://github.com/espressif/esp-sr)
- [WAV文件格式规范](http://soundfile.sapp.org/doc/WaveFormat/)
- [I2S协议详解 (B站)](https://search.bilibili.com/all?keyword=I2S%20%E9%9F%B3%E9%A2%91%E5%8D%8F%E8%AE%AE)
- [ESP32蓝牙音箱DIY (B站)](https://search.bilibili.com/all?keyword=ESP32%20%E8%93%9D%E7%89%99%E9%9F%B3%E7%AE%B1)

*最后更新：2026-05-27*
