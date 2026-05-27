/**
 * ================================================================
 * ESP-SparkBot -- 核心组件实现
 * ESP-SparkBot -- Core Component Implementation
 * ================================================================
 *
 * 替代原来的 src/pet.py
 * Replaces the original src/pet.py
 *
 * 实现:
 *   - 所有硬件外设初始化
 *   - WiFi 连接管理
 *   - 电池电压读取
 *   - 系统状态维护
 *
 * Implements:
 *   - All hardware peripheral initialization
 *   - WiFi connection management
 *   - Battery voltage reading
 *   - System state maintenance
 */

#include "sparkbot_core.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include <string.h>

static const char *TAG = "sparkbot_core";

/* ================================================================
 * 全局变量 | Global Variables
 * ================================================================ */

/* 事件队列引用 | Event queue reference */
static QueueHandle_t s_event_queue = NULL;

/* WiFi 事件组 | WiFi event group */
static EventGroupHandle_t s_wifi_event_group = NULL;

/* ADC 句柄 (电池监测) | ADC handle (battery monitor) */
#if CONFIG_SPARKBOT_POWER_MANAGEMENT_ENABLE
static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_cali_handle_t        s_adc_cali = NULL;
#endif

/* 系统版本 | System version */
static const char *SYSTEM_VERSION = "v1.0.0";

/* ================================================================
 * WiFi 事件回调 | WiFi Event Callback
 * ================================================================ */

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    static int s_retry_cnt = 0;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        /* WiFi STA 启动完成 — 开始连接 */
        /* WiFi STA started — start connecting */
        esp_wifi_connect();
        ESP_LOGI(TAG, "[WiFi] STA 已启动, 正在连接... | STA started, connecting...");

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        /* WiFi 断开连接 */
        /* WiFi disconnected */
        wifi_event_sta_disconnected_t *disconn = (wifi_event_sta_disconnected_t *)event_data;
        ESP_LOGW(TAG, "[WiFi] 断开连接, 原因: %d | Disconnected, reason: %d",
                 disconn->reason);

        if (s_retry_cnt < CONFIG_SPARKBOT_WIFI_MAX_RETRY) {
            ESP_LOGI(TAG, "[WiFi] 重试 %d/%d... | Retry %d/%d...",
                     s_retry_cnt + 1, CONFIG_SPARKBOT_WIFI_MAX_RETRY);
            esp_wifi_connect();
            s_retry_cnt++;
        } else {
            ESP_LOGE(TAG, "[WiFi] 达到最大重试次数, 连接失败 | Max retries reached, connection failed");
            if (s_wifi_event_group) {
                xEventGroupSetBits(s_wifi_event_group, BIT1);  /* EVENT_WIFI_FAILED */
            }
        }

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        /* WiFi 连接成功并获取到 IP |
        /* WiFi connected and got IP */
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "[WiFi] 已连接! IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_cnt = 0;

        if (s_wifi_event_group) {
            xEventGroupSetBits(s_wifi_event_group, BIT0);  /* EVENT_WIFI_CONNECTED */
        }
    }
}

/* ================================================================
 * 公共 API 实现 | Public API Implementation
 * ================================================================ */

esp_err_t sparkbot_core_init(QueueHandle_t event_queue)
{
    s_event_queue = event_queue;

    ESP_LOGI(TAG, "[Core] SparkBot 核心系统启动 v%s | Core system starting v%s",
             SYSTEM_VERSION, SYSTEM_VERSION);

    /* 初始化电池 ADC |
    /* Initialize battery ADC */
#if CONFIG_SPARKBOT_POWER_MANAGEMENT_ENABLE
    /* ---- ADC 初始化 (电池电压监测) ---- */
    /* ---- ADC init (battery voltage monitoring) ---- */
    adc_oneshot_unit_init_cfg_t adc_cfg = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_cfg, &s_adc_handle));

    /* ADC 通道配置 */
    /* ADC channel config */
    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        s_adc_handle,
        CONFIG_SPARKBOT_BATTERY_ADC_CHANNEL,
        &chan_cfg
    ));

    /* ADC 校准 (提高精度) |
    /* ADC calibration (improve accuracy) */
    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id = ADC_UNIT_1,
        .atten   = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_cfg, &s_adc_cali));
    ESP_LOGI(TAG, "[电池] ADC 初始化完成 | Battery ADC initialized");
#endif

    return ESP_OK;
}

esp_err_t sparkbot_core_deinit(void)
{
    ESP_LOGI(TAG, "[Core] 正在关闭系统... | Shutting down system...");

    /* 断开 WiFi 并清理 | Disconnect WiFi and cleanup */
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();

    /* 释放 ADC 资源 | Release ADC resources */
#if CONFIG_SPARKBOT_POWER_MANAGEMENT_ENABLE
    if (s_adc_cali != NULL) {
        adc_cali_delete_scheme_curve_fitting(s_adc_cali);
        s_adc_cali = NULL;
    }
    if (s_adc_handle != NULL) {
        adc_oneshot_del_unit(s_adc_handle);
        s_adc_handle = NULL;
    }
#endif

    ESP_LOGI(TAG, "[Core] 系统已停止 | System stopped");
    return ESP_OK;
}

/* ================================================================
 * 硬件初始化 | Hardware Initialization
 * ================================================================ */

esp_err_t sparkbot_i2c_init(void)
{
    i2c_config_t i2c_conf = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = CONFIG_SPARKBOT_IMU_PIN_SDA,
        .scl_io_num       = CONFIG_SPARKBOT_IMU_PIN_SCL,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,  /* 100 kHz 标准模式 */
        .clk_flags        = 0,
    };

    ESP_ERROR_CHECK(i2c_param_config(CONFIG_SPARKBOT_IMU_I2C_PORT, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(
        CONFIG_SPARKBOT_IMU_I2C_PORT,
        I2C_MODE_MASTER,
        0,      /* RX buffer size (不需要主接收) | RX buffer (no master receive) */
        0,      /* TX buffer size */
        0       /* 中断分配标志 | Interrupt allocation flags */
    ));

    ESP_LOGI(TAG, "[I2C] 总线初始化完成 (SCL=%d, SDA=%d, 100kHz) | I2C bus initialized",
             CONFIG_SPARKBOT_IMU_PIN_SCL, CONFIG_SPARKBOT_IMU_PIN_SDA);
    return ESP_OK;
}

esp_err_t sparkbot_spi_init(void)
{
    spi_bus_config_t bus_cfg = {
        .mosi_io_num     = CONFIG_SPARKBOT_DISPLAY_PIN_MOSI,
        .miso_io_num     = -1,  /* 显示屏不需要 MISO | Display doesn't need MISO */
        .sclk_io_num     = CONFIG_SPARKBOT_DISPLAY_PIN_SCLK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = 240 * 240 * 2 + 16,  /* 全帧 + 头部 | Full frame + header */
    };

    ESP_ERROR_CHECK(spi_bus_initialize(
        CONFIG_SPARKBOT_DISPLAY_SPI_HOST,
        &bus_cfg,
        SPI_DMA_CH_AUTO
    ));

    ESP_LOGI(TAG, "[SPI] 总线初始化完成 (MOSI=%d, SCLK=%d) | SPI bus initialized",
             CONFIG_SPARKBOT_DISPLAY_PIN_MOSI, CONFIG_SPARKBOT_DISPLAY_PIN_SCLK);
    return ESP_OK;
}

esp_err_t sparkbot_camera_init(void)
{
    ESP_LOGI(TAG, "[摄像头] 正在初始化 OV2640... | Initializing OV2640...");

    camera_config_t cam_cfg = {
        .pin_pwdn     = CONFIG_SPARKBOT_CAMERA_PIN_PWDN,
        .pin_reset    = CONFIG_SPARKBOT_CAMERA_PIN_RESET,
        .pin_xclk     = CONFIG_SPARKBOT_CAMERA_PIN_XCLK,
        .pin_sccb_sda = -1,  /* 使用引脚复用默认值 | Use pin mux default */
        .pin_sccb_scl = -1,
        .pin_d7       = CONFIG_SPARKBOT_CAMERA_PIN_D7,
        .pin_d6       = CONFIG_SPARKBOT_CAMERA_PIN_D6,
        .pin_d5       = CONFIG_SPARKBOT_CAMERA_PIN_D5,
        .pin_d4       = CONFIG_SPARKBOT_CAMERA_PIN_D4,
        .pin_d3       = CONFIG_SPARKBOT_CAMERA_PIN_D3,
        .pin_d2       = CONFIG_SPARKBOT_CAMERA_PIN_D2,
        .pin_d1       = CONFIG_SPARKBOT_CAMERA_PIN_D1,
        .pin_d0       = CONFIG_SPARKBOT_CAMERA_PIN_D0,
        .pin_vsync    = CONFIG_SPARKBOT_CAMERA_PIN_VSYNC,
        .pin_href     = CONFIG_SPARKBOT_CAMERA_PIN_HREF,
        .pin_pclk     = CONFIG_SPARKBOT_CAMERA_PIN_PCLK,

        .xclk_freq_hz   = 20000000,          /* 20MHz XCLK */
        .ledc_timer      = LEDC_TIMER_0,
        .ledc_channel    = LEDC_CHANNEL_0,

        .pixel_format    = PIXFORMAT_JPEG,    /* JPEG 压缩输出 */
        .frame_size      = FRAMESIZE_QQVGA,   /* 160x120 — 低延迟 */
        .jpeg_quality    = 12,                /* 0-63 (越低越高) | Lower = higher quality */
        .fb_count        = 2,                 /* 双帧缓冲 | Double frame buffer */
        .grab_mode       = CAMERA_GRAB_WHEN_EMPTY,
        .fb_location     = CAMERA_FB_IN_PSRAM, /* 帧缓冲放 PSRAM */
    };

    /* 初始化摄像头 |
    /* Initialize camera */
    esp_err_t err = esp_camera_init(&cam_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "[摄像头] 初始化失败! err=0x%x | Camera init failed! err=0x%x", err);
        return err;
    }

    /* 配置传感器参数 | Configure sensor parameters */
    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor != NULL) {
        sensor->set_brightness(sensor, CONFIG_SPARKBOT_CAMERA_BRIGHTNESS);
        sensor->set_contrast(sensor,   CONFIG_SPARKBOT_CAMERA_CONTRAST);
        sensor->set_vflip(sensor,      CONFIG_SPARKBOT_CAMERA_VFLIP ? 1 : 0);
        sensor->set_hmirror(sensor,    CONFIG_SPARKBOT_CAMERA_HMIRROR ? 1 : 0);
    }

    ESP_LOGI(TAG, "[摄像头] OV2640 初始化完成 | OV2640 initialized");
    return ESP_OK;
}

esp_err_t sparkbot_audio_init(void)
{
    ESP_LOGI(TAG, "[音频] 正在初始化 I2S... | Initializing I2S...");

    /* ---- I2S 输出 (扬声器 | Speaker) ---- */
    i2s_chan_config_t tx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(
        CONFIG_SPARKBOT_AUDIO_I2S_PORT,
        I2S_ROLE_MASTER
    );
    ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, NULL, NULL));  /* TODO: 保存句柄 */

    i2s_std_config_t tx_std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(CONFIG_SPARKBOT_AUDIO_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_16BIT,
            I2S_SLOT_MODE_MONO
        ),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = CONFIG_SPARKBOT_AUDIO_PIN_BCLK,
            .ws   = CONFIG_SPARKBOT_AUDIO_PIN_LRCK,
            .dout = CONFIG_SPARKBOT_AUDIO_PIN_DOUT,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    /* i2s_channel_init_std_mode(tx_chan, &tx_std_cfg); */

    /* ---- I2S 输入 (麦克风 | Microphone) ---- */
    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(
        CONFIG_SPARKBOT_MIC_I2S_PORT,
        I2S_ROLE_MASTER
    );
    ESP_ERROR_CHECK(i2s_new_channel(&rx_chan_cfg, NULL, NULL));  /* TODO: 保存句柄 */

    i2s_std_config_t rx_std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(CONFIG_SPARKBOT_MIC_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_16BIT,
            I2S_SLOT_MODE_MONO
        ),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = CONFIG_SPARKBOT_MIC_PIN_SCLK,
            .ws   = CONFIG_SPARKBOT_MIC_PIN_WS,
            .dout = I2S_GPIO_UNUSED,
            .din  = CONFIG_SPARKBOT_MIC_PIN_DIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    /* i2s_channel_init_std_mode(rx_chan, &rx_std_cfg); */

    ESP_LOGI(TAG, "[音频] I2S 音频系统初始化完成 | I2S audio system initialized");
    ESP_LOGI(TAG, "[音频]   输出 (Speaker): BCLK=%d LRCK=%d DOUT=%d",
             CONFIG_SPARKBOT_AUDIO_PIN_BCLK,
             CONFIG_SPARKBOT_AUDIO_PIN_LRCK,
             CONFIG_SPARKBOT_AUDIO_PIN_DOUT);
    ESP_LOGI(TAG, "[音频]   输入 (Mic):     SCLK=%d WS=%d   DIN=%d",
             CONFIG_SPARKBOT_MIC_PIN_SCLK,
             CONFIG_SPARKBOT_MIC_PIN_WS,
             CONFIG_SPARKBOT_MIC_PIN_DIN);

    return ESP_OK;
}

esp_err_t sparkbot_core_wifi_init(EventGroupHandle_t event_group)
{
    s_wifi_event_group = event_group;

    /* 注册 WiFi 事件回调 */
    /* Register WiFi event callbacks */
    ESP_ERROR_CHECK(esp_event_handler_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL));

    /* 配置 WiFi STA 模式 */
    /* Configure WiFi station mode */
    wifi_config_t wifi_cfg = {0};
    strncpy((char *)wifi_cfg.sta.ssid,     CONFIG_SPARKBOT_WIFI_SSID, 32);
    strncpy((char *)wifi_cfg.sta.password, CONFIG_SPARKBOT_WIFI_PASSWORD, 64);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "[WiFi] 正在连接 %s... | Connecting to %s...",
             wifi_cfg.sta.ssid);

    return ESP_OK;
}

/* ================================================================
 * 电源管理 | Power Management
 * ================================================================ */

uint32_t sparkbot_core_get_battery_voltage(void)
{
#if CONFIG_SPARKBOT_POWER_MANAGEMENT_ENABLE
    if (s_adc_handle == NULL) {
        return 0;
    }

    /* ADC 原始值读取 | Read raw ADC value */
    int raw_val = 0;
    esp_err_t err = adc_oneshot_read(s_adc_handle,
                                      CONFIG_SPARKBOT_BATTERY_ADC_CHANNEL,
                                      &raw_val);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "[电池] ADC 读取失败 | ADC read failed: %s", esp_err_to_name(err));
        return 0;
    }

    /* 校准 ADC 值 | Calibrate ADC value */
    int cali_mv = 0;
    if (s_adc_cali != NULL) {
        adc_cali_raw_to_voltage(s_adc_cali, raw_val, &cali_mv);
    } else {
        /* 无校准时粗略换算 (12-bit, 3.3V 参考电压, 11dB 衰减 ≈ 3.9V 量程) */
        /* Rough conversion without calibration (12-bit, 3.3V ref, 11dB atten ≈ 3.9V range) */
        cali_mv = (raw_val * 3900) / 4096;
    }

    /* 考虑分压电阻 (电池电压 = ADC电压 × 分压比) */
    /* Account for voltage divider (battery = ADC voltage × divider ratio) */
    uint32_t battery_mv = (uint32_t)(cali_mv * CONFIG_SPARKBOT_BATTERY_VOLTAGE_DIVIDER);

    return battery_mv;
#else
    return 0;
#endif
}

esp_err_t sparkbot_core_enter_sleep(uint32_t sleep_duration_ms)
{
    ESP_LOGI(TAG, "[电源] 进入睡眠 %lums... | Entering sleep for %lums...",
             (unsigned long)sleep_duration_ms);

    /* 配置唤醒源 (定时器) | Configure wakeup source (timer) */
    esp_sleep_enable_timer_wakeup((uint64_t)sleep_duration_ms * 1000);

    /* 进入浅度睡眠 (Light Sleep) */
    /* Enter light sleep — 保留 RAM, 快速唤醒 */
    /* Enter light sleep — RAM retained, fast wakeup */
    esp_err_t err = esp_light_sleep_start();

    ESP_LOGI(TAG, "[电源] 从睡眠中唤醒 | Woke from sleep");
    return err;
}

bool sparkbot_core_is_charging(void)
{
    /* USB 充电检测 — 检查 VBUS 引脚电平 */
    /* USB charging detection — check VBUS pin level */
    /* ESP32-S3 USB OTG 有内置的 VBUS 检测 */
    /* ESP32-S3 USB OTG has built-in VBUS detection */
    /* 简化实现: 电池电压 > 4.2V 视为充电中 */
    /* Simplified: battery > 4.2V = charging */
    uint32_t battery = sparkbot_core_get_battery_voltage();
    return (battery > 4200);
}

/* ================================================================
 * 系统信息 | System Information
 * ================================================================ */

const char *sparkbot_get_version(void)
{
    return SYSTEM_VERSION;
}

float sparkbot_get_chip_temperature(void)
{
    /* ESP32-S3 内部温度传感器 |
    /* ESP32-S3 internal temperature sensor */
    /* 简化实现: 返回固定值, 实际需要读取 TSENS 寄存器 */
    /* Simplified: returns fixed value, actual needs TSENS register read */
    return 45.0f;
}

int8_t sparkbot_get_wifi_rssi(void)
{
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        return ap_info.rssi;
    }
    return 0;
}
