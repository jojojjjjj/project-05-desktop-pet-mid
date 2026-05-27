# Day 11: 综合功能集成与调试 | System Integration & Debugging

> **今日目标:**
> - 整合所有功能模块为一个完整系统
> - 实现小游戏功能 (木鱼、骰子、2048)
> - 了解ESP-NOW多设备同步
> - 实现USB投屏副屏功能
> - 进行系统稳定性测试与功耗优化
>
> **产出:** 一个功能完整的SparkBot系统，所有模块协调工作，运行稳定

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间活动 | AI对话功能展示、问题汇总 |
| 09:15 - 10:30 | 知识讲解 | 系统架构设计、模块间通信、电源管理 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | 系统整合、任务优先级优化 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | 小游戏开发、ESP-NOW同步 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 稳定性测试、功耗优化、USB投屏 |
| 16:30 - 17:00 | 总结分享 | 系统演示、整合经验讨论 |

---

## 上午: 系统整合 | Morning: System Integration

### 为什么要学这个? | Why Learn This?

到目前为止，你的代码分散在多个文件中——传感器读取、屏幕显示、WiFi天气、AI对话各自为政。今天把它们整合为一个协同工作的系统。系统整合是软件工程中最重要的环节——一个项目中80%的时间可能花在"让各部分协同工作"上。

So far your code is scattered across multiple files. Today we integrate them into one cooperative system. System integration is one of the most critical software engineering phases -- 80% of project time may go into "making things work together".

### 任务11.1: 系统架构设计 (30分钟)

**SparkBot系统架构:**

```
┌─────────────────────────────────────────────────────┐
│                    SparkBot 系统                      │
├─────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────┐        │
│  │         LVGL UI 任务 (优先级2)           │        │
│  │  时钟页 | 天气页 | 对话页 | 游戏页       │        │
│  └──────────────────┬──────────────────────┘        │
│                     │ 消息总线(队列+事件组)           │
│  ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐│
│  │传感器 │ │WiFi/  │ │音频   │ │摄像头 │ │蓝牙   ││
│  │任务(1)│ │HTTP(1)│ │任务(1)│ │任务(1)│ │任务(3)││
│  │BMI270 │ │LLM API│ │ES8311 │ │OV2640 │ │A2DP   ││
│  │触摸   │ │天气API│ │录音播放│ │人脸检测│ │       ││
│  └───────┘ └───────┘ └───────┘ └───────┘ └───────┘│
│                                                     │
│  FreeRTOS 调度器 (优先级: IDLE=0, 最高=24)           │
└─────────────────────────────────────────────────────┘
```

**任务优先级分配:**

| 任务 | 优先级 | 栈大小 | 原因 |
|------|--------|--------|------|
| LVGL UI刷新 | 2 | 8KB | UI流畅但不能阻塞其他任务 |
| 音频播放/录音 | 3 | 4KB | 音频需低延迟 |
| WiFi/HTTP/LLM | 1 | 8KB | HTTP慢，低优先级 |
| 传感器采集 | 1 | 2KB | 周期采集，不紧急 |
| 蓝牙A2DP | 3 | 6KB | 音频实时性要求 |
| 系统监控(IDLE) | 0 | 1KB | 统计CPU/内存 |

**步骤:**

```c
// sparkbot_main.c -- 系统主控
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

static const char *TAG = "SYSTEM";

// 系统状态
typedef enum {
    STATE_BOOT,         // 启动中
    STATE_IDLE,         // 待机(显示时钟)
    STATE_WEATHER,      // 天气页
    STATE_AI_DIALOG,    // AI对话中
    STATE_GAME,         // 游戏中
    STATE_SLEEP,        // 休眠
} system_state_t;

static system_state_t g_state = STATE_BOOT;

void app_main(void)
{
    ESP_LOGI(TAG, "=== ESP-SparkBot 系统启动 ===");
    ESP_LOGI(TAG, "固件版本: v1.0.0");
    
    // ---- 阶段1: 基础硬件 ----
    ESP_LOGI(TAG, "[1/6] 初始化基础硬件...");
    nvs_flash_init();
    backlight_init();
    lcd_init();
    lvgl_init(panel);
    create_main_ui();
    
    // ---- 阶段2: 传感器 ----
    ESP_LOGI(TAG, "[2/6] 初始化传感器...");
    i2c_master_init();      // I2C总线
    bmi270_init();          // BMI270
    touch_button_init();    // 触摸按键
    
    // ---- 阶段3: 音频 ----
    ESP_LOGI(TAG, "[3/6] 初始化音频...");
    i2s_init();             // I2S
    es8311_init();          // 音频编解码器
    
    // ---- 阶段4: 摄像头 ----
    ESP_LOGI(TAG, "[4/6] 初始化摄像头...");
    // camera_init();       // 可能失败，降级运行
    
    // ---- 阶段5: 网络 ----
    ESP_LOGI(TAG, "[5/6] 初始化网络...");
    if (wifi_init_sta() == ESP_OK) {
        ntp_time_init();
        xTaskCreate(weather_task, "weather", 8192, NULL, 1, NULL);
        xTaskCreate(ai_dialog_task, "ai", 8192, NULL, 1, NULL);
    } else {
        ESP_LOGW(TAG, "WiFi不可用，离线运行");
    }
    
    // ---- 阶段6: 启动任务 ----
    ESP_LOGI(TAG, "[6/6] 启动任务...");
    xTaskCreate(sensor_task, "sensor", 2048, NULL, 1, NULL);
    xTaskCreate(touch_task, "touch", 2048, NULL, 2, NULL);
    xTaskCreate(system_monitor, "sysmon", 2048, NULL, 0, NULL);
    
    ESP_LOGI(TAG, "=== SparkBot 启动完成! ===");
    ESP_LOGI(TAG, "空闲内存: %d 字节", esp_get_free_heap_size());
}
```

### 任务11.2: 状态管理与触摸分发 (30分钟)

```c
// 状态切换
static void system_set_state(system_state_t new_state)
{
    ESP_LOGI(TAG, "状态: %d -> %d", g_state, new_state);
    g_state = new_state;
    
    switch (new_state) {
        case STATE_IDLE:     lv_scr_load(clock_screen); break;
        case STATE_WEATHER:  lv_scr_load(weather_screen); break;
        case STATE_AI_DIALOG:lv_scr_load(dialog_screen); break;
        case STATE_GAME:     lv_scr_load(game_screen); break;
        case STATE_SLEEP:    backlight_set(0); break;
        default: break;
    }
}

// 触摸事件分发
static void touch_event_handler(int event_type)
{
    switch (g_state) {
        case STATE_IDLE:
            if (event_type == 1) system_set_state(STATE_WEATHER);   // 单击切换
            if (event_type == 3) system_set_state(STATE_AI_DIALOG); // 长按对话
            break;
        case STATE_WEATHER:
            if (event_type == 1) system_set_state(STATE_IDLE); // 单击返回
            break;
        case STATE_AI_DIALOG:
            if (event_type == 3) system_set_state(STATE_IDLE); // 长按退出
            break;
        // ... 其他状态
    }
}
```

---

## 下午: 小游戏与高级功能 | Afternoon: Minigames & Advanced Features

### 任务11.3: 电子木鱼 (30分钟)

```c
// 电子木鱼 -- 使用BMI270检测敲击
static int merit_count = 0;
static lv_obj_t *merit_label;

static void wooden_fish_detect_task(void *pvParameters)
{
    float ax, ay, az, last_az = 0;
    
    while (1) {
        bmi270_read_accel(&ax, &ay, &az);
        
        // 检测Z轴加速度突变 (快速敲击 > 2g)
        float delta = az - last_az;
        if (delta > 2.0f) {
            merit_count++;
            
            char buf[32];
            snprintf(buf, sizeof(buf), "功德: %d", merit_count);
            lv_label_set_text(merit_label, buf);
            
            // 木鱼弹跳动画
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, merit_label);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_values(&a, lv_obj_get_y(merit_label), 
                               lv_obj_get_y(merit_label) - 10);
            lv_anim_set_time(&a, 50);
            lv_anim_set_playback_time(&a, 50);
            lv_anim_start(&a);
            
            // 播放音效
            // play_sound("muyu");
            
            ESP_LOGI(TAG, "功德+1! 总计: %d", merit_count);
        }
        last_az = az;
        vTaskDelay(pdMS_TO_TICKS(10));  // 100Hz检测
    }
}
```

### 任务11.4: 骰子游戏 (20分钟)

```c
// 骰子 -- 倾斜SparkBot投骰子
static void dice_roll(void)
{
    int result = (esp_random() % 6) + 1;
    char buf[32];
    snprintf(buf, sizeof(buf), "%d 点!", result);
    
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, buf);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_36, 0);
    lv_obj_center(label);
    
    // 摇晃动画
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, label);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_values(&a, -30, 30);
    lv_anim_set_time(&a, 100);
    lv_anim_set_repeat_count(&a, 5);
    lv_anim_set_playback_time(&a, 100);
    lv_anim_start(&a);
    
    ESP_LOGI(TAG, "骰子: %d 点!", result);
}
```

### 任务11.5: ESP-NOW多设备同步 (30分钟)

ESP-NOW是乐鑫的私有无线协议，无需路由器，设备间直接通信，延迟<3ms。

```c
#include "esp_now.h"

typedef struct __attribute__((packed)) {
    uint8_t type;       // 消息类型: 1=木鱼 2=骰子 3=心情
    uint32_t value;     // 数据
} espnow_msg_t;

static const uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static void espnow_recv_cb(const esp_now_recv_info_t *recv_info,
                            const uint8_t *data, int len)
{
    espnow_msg_t *msg = (espnow_msg_t *)data;
    switch (msg->type) {
        case 1: ESP_LOGI(TAG, "ESP-NOW: 收到功德广播! 值=%lu", msg->value);
                // 同步显示对方敲击
                break;
        case 2: ESP_LOGI(TAG, "ESP-NOW: 收到骰子结果! 值=%lu", msg->value);
                break;
    }
}

void espnow_init(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    
    esp_now_init();
    esp_now_register_recv_cb(espnow_recv_cb);
    
    esp_now_peer_info_t peer = { .channel = 0, .encrypt = false };
    memcpy(peer.peer_addr, broadcast_mac, 6);
    esp_now_add_peer(&peer);
    
    ESP_LOGI(TAG, "ESP-NOW就绪 (广播模式)");
}

// 广播消息
void espnow_broadcast(uint8_t type, uint32_t value)
{
    espnow_msg_t msg = { .type = type, .value = value };
    esp_now_send(broadcast_mac, (uint8_t *)&msg, sizeof(msg));
}
```

### 任务11.6: 系统稳定性与功耗优化 (20分钟)

**稳定性检查清单:**

```
[ ] 连续运行1小时不死机
[ ] WiFi断线自动重连
[ ] 无内存泄漏 (用 esp_get_free_heap_size() 定时打印)
[ ] 无栈溢出 (用 uxTaskGetStackHighWaterMark() 检查)
[ ] 看门狗不误触发
```

**系统监控任务:**

```c
static void system_monitor(void *pvParameters)
{
    while (1) {
        ESP_LOGI(TAG, "===== 系统状态 =====");
        ESP_LOGI(TAG, "状态: %d", g_state);
        ESP_LOGI(TAG, "空闲DRAM: %d 字节", 
                 heap_caps_get_free_size(MALLOC_CAP_8BIT));
        ESP_LOGI(TAG, "空闲PSRAM: %d 字节",
                 heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGI(TAG, "历史最小DRAM: %d 字节",
                 heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT));
        
        UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI(TAG, "当前栈水位: %d 字 (剩余)", watermark);
        
        vTaskDelay(pdMS_TO_TICKS(30000));  // 30秒报告一次
    }
}
```

**功耗优化策略:**

```c
// 空闲时降低频率和关闭背光
void enter_low_power(void)
{
    esp_pm_config_t pm_cfg = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 80,        // 降低最低频率
        .light_sleep_enable = true,
    };
    esp_pm_configure(&pm_cfg);
}

void auto_sleep_check(void)
{
    static int idle_counter = 0;
    if (g_state == STATE_IDLE) {
        if (++idle_counter > 300) {  // 5分钟无操作
            system_set_state(STATE_SLEEP);
            backlight_set(0);
        }
    } else {
        idle_counter = 0;
    }
}
```

**USB投屏副屏 (简版):**

通过USB Serial/JTAG将屏幕帧数据传输到PC端Python脚本显示：

```c
#include "driver/usb_serial_jtag.h"

static void usb_screen_task(void *pvParameters)
{
    while (1) {
        // 从LVGL获取当前帧缓冲
        // 通过USB发送到PC
        // usb_serial_jtag_write_bytes(fb, fb_size, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(33));  // ~30fps
    }
}
```

---

## 今日作业 | Homework

### 必做题
1. 完成系统整合: 传感器 + 显示 + WiFi天气 + AI对话协调运行
2. 实现基本的状态切换逻辑 (至少3个状态)
3. 完成至少一个小游戏 (木鱼或骰子)
4. 运行系统监控任务，打印空闲内存和栈水位

### 挑战题
1. 实现ESP-NOW两台SparkBot通信
2. 完成2048游戏 (倾斜控制方向)
3. 实现USB副屏功能基础框架

### 思考题
1. 为什么系统整合往往是项目中最困难的部分？你遇到了哪些"坑"？
2. FreeRTOS任务优先级设置不当会导致什么问题？
3. 嵌入式系统的内存管理有什么特殊之处？为什么要关注栈水位？

---

## 明日预告 | Tomorrow's Preview

明天是最后一天！你将向全班展示你的ESP-SparkBot作品。准备好你的演示、部署项目网站到GitHub Pages，撰写技术总结报告。这是你12天所有努力的高光时刻！

Tomorrow is the final day! Present your ESP-SparkBot to the class. Deploy your project website, write your technical report. The highlight of all your 12-day effort!

---

## 参考资源 | References

- [ESP-NOW 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/network/esp_now.html)
- [ESP-IDF 电源管理](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/system/power_management.html)
- [ESP-IDF USB Serial/JTAG](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/peripherals/usb_serial_jtag.html)
- [FreeRTOS 任务水位检测](https://www.freertos.org/uxTaskGetStackHighWaterMark.html)
- [LVGL 2048 游戏示例](https://github.com/lvgl/lv_demos)

*最后更新：2026-05-27*
