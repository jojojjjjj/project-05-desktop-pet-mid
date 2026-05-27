# Day 09: WiFi联网与天气时钟 | WiFi & Weather Clock

> **今日目标:**
> - 掌握ESP32-S3 WiFi Station模式连接
> - 使用HTTP Client获取天气API数据
> - 实现NTP时间同步
> - 学习JSON解析 (cJSON)
> - 完成天气时钟功能整合
>
> **产出:** SparkBot连接WiFi，同步网络时间，获取真实天气数据，在LVGL界面上显示完整天气时钟

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间活动 | 音频系统验收、蓝牙音箱演示 |
| 09:15 - 10:30 | 知识讲解 | WiFi协议栈、HTTP协议、JSON格式 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | WiFi连接、NTP时间同步 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | HTTP Client获取天气、JSON解析 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 天气时钟整合到LVGL、多API接入 |
| 16:30 - 17:00 | 总结分享 | 天气时钟成果展示 |

---

## 上午: WiFi连接与NTP时间 | Morning: WiFi Connection & NTP Time

### 为什么要学这个? | Why Learn This?

WiFi是物联网设备最重要的联网方式。没有WiFi，SparkBot只能是一个离线玩具；有了WiFi，它就能获取时间、天气、新闻、调用AI大模型——变成一个真正的"智能"设备。

WiFi is the most important networking method for IoT devices. Without WiFi, SparkBot is just an offline toy; with WiFi, it can get time, weather, news, and call AI models -- becoming a truly "smart" device.

HTTP是互联网最基础的通信协议——你每天用的浏览器、微信、抖音，底层都是HTTP。掌握嵌入式HTTP客户端编程，你的设备就能和世界上任何一个服务器通信。

HTTP is the most fundamental internet communication protocol. Mastering embedded HTTP client programming lets your device communicate with any server in the world.

### 任务9.1: WiFi Station模式连接 (40分钟)

**重要: ESP32-S3仅支持2.4GHz WiFi，不支持5GHz频段！**

ESP32-S3 supports only 2.4GHz WiFi, NOT 5GHz!

**步骤:**

1. **创建WiFi管理代码**

```c
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "WIFI";
static EventGroupHandle_t wifi_event_group;
static int s_retry_num = 0;

#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

// WiFi配置 -- 从menuconfig或配置文件读取
#define WIFI_SSID       "你的WiFi名"
#define WIFI_PASSWORD   "你的WiFi密码"

// WiFi事件处理
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 5) {
            ESP_LOGW(TAG, "WiFi断开, 重试 %d/5...", s_retry_num + 1);
            esp_wifi_connect();
            s_retry_num++;
        } else {
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "获取IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// WiFi初始化
esp_err_t wifi_init_sta(void)
{
    nvs_flash_init();
    wifi_event_group = xEventGroupCreate();
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "WiFi初始化完成, 连接中: %s", WIFI_SSID);
    
    // 等待连接结果 (最多30秒)
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(30000));
    
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "WiFi连接成功!");
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "WiFi连接失败!");
        return ESP_FAIL;
    }
}
```

**预期结果:**
- 串口显示WiFi连接过程: 启动 -> 连接AP -> 获取IP
- 最终打印IP地址(如192.168.1.100)

**常见问题:**
- **连接超时**：检查WiFi密码、信号强度、确认是2.4GHz WiFi
- **反复断开重连**：电源不稳定可能导致WiFi掉线，检查DC-DC供电
- **获取不到IP**：路由器DHCP问题

### 任务9.2: NTP时间同步 (30分钟)

```c
#include "esp_sntp.h"
#include <time.h>

static const char *TAG = "TIME";

// NTP时间同步
static void ntp_time_init(void)
{
    ESP_LOGI(TAG, "正在同步网络时间...");
    
    // 设置时区为中国标准时间 (UTC+8)
    setenv("TZ", "CST-8", 1);
    tzset();
    
    // 配置NTP服务器 (国内推荐阿里云和国家授时中心)
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "ntp.aliyun.com");
    esp_sntp_setservername(1, "ntp.ntsc.ac.cn");
    esp_sntp_init();
    
    // 等待同步
    int retry = 0;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && retry < 15) {
        ESP_LOGI(TAG, "等待NTP同步... (%d/15)", retry + 1);
        vTaskDelay(pdMS_TO_TICKS(2000));
        retry++;
    }
    
    if (retry < 15) {
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        ESP_LOGI(TAG, "NTP同步成功! %d-%02d-%02d %02d:%02d:%02d",
                 timeinfo.tm_year + 1900, timeinfo.tm_mon + 1,
                 timeinfo.tm_mday, timeinfo.tm_hour,
                 timeinfo.tm_min, timeinfo.tm_sec);
    } else {
        ESP_LOGE(TAG, "NTP同步失败!");
    }
}

// 获取格式化的时间和日期字符串
static void get_time_str(char *buf, size_t len)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(buf, len, "%H:%M", &timeinfo);
}

static void get_date_str(char *buf, size_t len)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    const char *wd[] = {"周日","周一","周二","周三","周四","周五","周六"};
    strftime(buf, len, "%Y年%m月%d日", &timeinfo);
    strcat(buf, " ");
    strcat(buf, wd[timeinfo.tm_wday]);
}
```

---

## 下午: HTTP Client与天气API | Afternoon: HTTP Client & Weather API

### 任务9.3: HTTP Client获取天气数据 (40分钟)

```c
#include "esp_http_client.h"
#include "cJSON.h"

static const char *TAG = "WEATHER";

// 心知天气API配置 (需注册: seniverse.com)
#define WEATHER_API_URL "https://api.seniverse.com/v3/weather/now.json"
#define WEATHER_API_KEY "你的API密钥"
#define WEATHER_CITY    "beijing"

// 天气数据结构
typedef struct {
    char location[32];
    char text[32];         // 晴/多云/雨
    int  temperature;
    int  humidity;
    char code[8];
} weather_data_t;

// HTTP响应处理 (动态拼接)
typedef struct {
    char *buffer;
    int   length;
} http_response_t;

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    if (evt->event_id == HTTP_EVENT_ON_DATA) {
        http_response_t *resp = (http_response_t *)evt->user_data;
        char *new_buf = realloc(resp->buffer, resp->length + evt->data_len + 1);
        if (new_buf) {
            resp->buffer = new_buf;
            memcpy(resp->buffer + resp->length, evt->data, evt->data_len);
            resp->length += evt->data_len;
            resp->buffer[resp->length] = '\0';
        }
    }
    return ESP_OK;
}

// 获取天气
static esp_err_t fetch_weather(weather_data_t *weather)
{
    http_response_t resp = { .buffer = NULL, .length = 0 };
    
    char url[512];
    snprintf(url, sizeof(url),
             "%s?key=%s&location=%s&language=zh-Hans&unit=c",
             WEATHER_API_URL, WEATHER_API_KEY, WEATHER_CITY);
    
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 10000,
        .event_handler = http_event_handler,
        .user_data = &resp,
        .skip_cert_common_name_check = true,  // 开发阶段
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);
    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);
    
    ESP_LOGI(TAG, "HTTP状态: %d", status);
    
    if (status == 200 && resp.buffer) {
        parse_weather_json(resp.buffer, weather);
        free(resp.buffer);
        return ESP_OK;
    }
    
    free(resp.buffer);
    return ESP_FAIL;
}
```

### 任务9.4: JSON解析 -- cJSON (30分钟)

```c
// 解析心知天气JSON响应
esp_err_t parse_weather_json(const char *json_str, weather_data_t *weather)
{
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        ESP_LOGE(TAG, "JSON解析失败!");
        return ESP_FAIL;
    }
    
    cJSON *results = cJSON_GetObjectItem(root, "results");
    if (!cJSON_IsArray(results)) { cJSON_Delete(root); return ESP_FAIL; }
    
    cJSON *result = cJSON_GetArrayItem(results, 0);
    if (!result) { cJSON_Delete(root); return ESP_FAIL; }
    
    // 城市名
    cJSON *location = cJSON_GetObjectItem(result, "location");
    cJSON *name = cJSON_GetObjectItem(location, "name");
    if (name) strncpy(weather->location, name->valuestring, sizeof(weather->location) - 1);
    
    // 天气数据
    cJSON *now = cJSON_GetObjectItem(result, "now");
    cJSON *text = cJSON_GetObjectItem(now, "text");
    cJSON *temp = cJSON_GetObjectItem(now, "temperature");
    cJSON *code = cJSON_GetObjectItem(now, "code");
    cJSON *hum = cJSON_GetObjectItem(now, "humidity");
    
    if (text) strncpy(weather->text, text->valuestring, sizeof(weather->text) - 1);
    if (temp) weather->temperature = atoi(temp->valuestring);
    if (code) strncpy(weather->code, code->valuestring, sizeof(weather->code) - 1);
    if (hum)  weather->humidity = atoi(hum->valuestring);
    
    cJSON_Delete(root);
    
    ESP_LOGI(TAG, "天气: %s %s %d°C 湿度:%d%%",
             weather->location, weather->text,
             weather->temperature, weather->humidity);
    
    return ESP_OK;
}
```

### 任务9.5: 天气时钟整合到LVGL (30分钟)

```c
// 天气更新任务
static void weather_update_task(void *pvParameters)
{
    weather_data_t weather;
    char time_buf[16], date_buf[32];
    
    // 等待WiFi连接
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    ntp_time_init();
    
    while (1) {
        // 每秒更新时间
        get_time_str(time_buf, sizeof(time_buf));
        get_date_str(date_buf, sizeof(date_buf));
        lv_label_set_text(time_label, time_buf);
        lv_label_set_text(date_label, date_buf);
        
        // 每30分钟更新天气
        static int counter = 0;
        if (counter % 30 == 0 && fetch_weather(&weather) == ESP_OK) {
            char temp_str[16];
            snprintf(temp_str, sizeof(temp_str), "%d°C", weather.temperature);
            lv_label_set_text(temp_label, temp_str);
            lv_label_set_text(weather_desc, weather.text);
            lv_label_set_text(city_label, weather.location);
        }
        counter++;
        
        vTaskDelay(pdMS_TO_TICKS(60000));  // 每分钟刷新
    }
}
```

**常用的免费天气API:**

| API | 中文支持 | 免费额度 | 注册地址 |
|-----|---------|---------|----------|
| 心知天气 | 好 | 400次/小时 | seniverse.com |
| 和风天气 | 好 | 1000次/天 | dev.qweather.com |
| OpenWeatherMap | 一般 | 60次/分钟 | openweathermap.org |
| 高德天气 | 好 | 5000次/天 | lbs.amap.com |

**预期结果:**
- SparkBot屏幕实时显示时间 (时:分, 每分钟刷新)
- 显示日期和星期
- 显示天气图标、温度、城市名
- 每30分钟自动更新天气数据

**常见问题:**
- **天气API返回错误**：检查API Key是否正确，确认免费API调用次数未超限
- **JSON解析崩溃**：确认响应是合法JSON；打印原始响应调试
- **时间不对**：检查时区设置(CST-8)
- **HTTPS错误**：开发阶段用 `skip_cert_common_name_check = true`

---

## 今日作业 | Homework

### 必做题
1. 完成WiFi STA模式连接，串口打印成功连接的IP地址
2. 实现NTP时间同步，在屏幕上显示当前时间
3. 获取天气API数据并解析JSON，显示温度和天气描述
4. 将时间和天气整合到LVGL天气时钟界面

### 挑战题
1. 添加未来3天天气预报 (需不同的API端点)
2. 根据天气代码自动切换天气图标 (晴/多云/雨/雪)
3. 实现WiFi断线自动重连 + 界面显示连接状态

### 思考题
1. HTTP和HTTPS有什么区别？为什么ESP32-S3的HTTPS需要额外配置证书？
2. NTP时间同步的原理是什么？为什么网络时间比本地晶振更准确？
3. JSON为什么成为物联网数据交换的标准格式？

---

## 明日预告 | Tomorrow's Preview

明天是最激动人心的一天！我们将给SparkBot接入AI大模型，让它能真正"对话"。实现语音输入->文字识别->AI回复->TTS输出的完整流水线！

Tomorrow is the most exciting day! We integrate AI large models into SparkBot for real conversation. Full pipeline: voice -> ASR -> AI reply -> TTS!

---

## 参考资源 | References

- [ESP-IDF WiFi 驱动指南](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/network/esp_wifi.html)
- [ESP-IDF HTTP Client 指南](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/protocols/esp_http_client.html)
- [ESP-IDF SNTP 时间同步](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/system/system_time.html)
- [cJSON GitHub](https://github.com/DaveGamble/cJSON)
- [心知天气 API 文档](https://seniverse.yuque.com/hyper_data/api_v3)

*最后更新：2026-05-27*
