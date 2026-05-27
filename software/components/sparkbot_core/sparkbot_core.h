/**
 * ================================================================
 * ESP-SparkBot -- 核心组件 (硬件抽象层)
 * ESP-SparkBot -- Core Component (Hardware Abstraction Layer)
 * ================================================================
 *
 * 替代原来的 src/pet.py
 * Replaces the original src/pet.py
 *
 * 功能 (Functions):
 *   - 硬件抽象层 (HAL) — 封装所有外设初始化
 *   - 系统状态管理 | System state management
 *   - 事件系统 | Event system
 *   - 电源管理 | Power management
 *
 * 这是整个机器人固件的"大脑"组件。
 * This is the "brain" component of the entire robot firmware.
 */

#ifndef SPARKBOT_CORE_H
#define SPARKBOT_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_camera.h"
#include "driver/i2s.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * 系统事件定义 | System Event Definitions
 * ================================================================ */

/**
 * @brief SparkBot 系统事件类型枚举
 * SparkBot System Event Type Enumeration
 *
 * 所有跨任务通信都通过此事件系统。
 * All cross-task communication goes through this event system.
 */
typedef enum {
    /* ---- 触摸与输入事件 (Touch & Input Events) ---- */
    SPARKBOT_EVENT_TOUCH            = 0x10, /* 触摸检测 | Touch detected */
    SPARKBOT_EVENT_GESTURE          = 0x11, /* 手势识别 | Gesture recognized */
    SPARKBOT_EVENT_BUTTON           = 0x12, /* 按键按下 | Button pressed */

    /* ---- 音频事件 (Audio Events) ---- */
    SPARKBOT_EVENT_START_LISTENING  = 0x20, /* 开始聆听 | Start listening */
    SPARKBOT_EVENT_SPEECH_DETECTED  = 0x21, /* 检测到语音 | Speech detected */
    SPARKBOT_EVENT_SPEECH_RESULT    = 0x22, /* 语音识别结果 | ASR result */
    SPARKBOT_EVENT_TTS_PLAY         = 0x23, /* 播放 TTS | Play TTS */
    SPARKBOT_EVENT_TTS_COMPLETE     = 0x24, /* TTS 播放完成 | TTS complete */

    /* ---- AI 事件 (AI Events) ---- */
    SPARKBOT_EVENT_AI_CHAT_REQUEST  = 0x30, /* AI 对话请求 | AI chat request */
    SPARKBOT_EVENT_AI_CHAT_RESPONSE = 0x31, /* AI 对话回复 | AI chat response */
    SPARKBOT_EVENT_AI_CLEAR_HISTORY = 0x32, /* 清除对话历史 | Clear chat history */

    /* ---- 系统事件 (System Events) ---- */
    SPARKBOT_EVENT_LOW_BATTERY      = 0x40, /* 电池电量低 | Low battery */
    SPARKBOT_EVENT_CHARGING         = 0x41, /* USB 插入充电 | USB plugged */
    SPARKBOT_EVENT_UNPLUGGED        = 0x42, /* USB 拔出 | USB unplugged */
    SPARKBOT_EVENT_ERROR            = 0x43, /* 系统错误 | System error */
    SPARKBOT_EVENT_SHUTDOWN         = 0x44, /* 系统关机 | System shutdown */

    /* ---- 自定义事件 (Custom Events) ---- */
    SPARKBOT_EVENT_CUSTOM_BASE      = 0x80, /* 用户自定义事件基值 | Custom event base */
} sparkbot_event_type_t;

/**
 * @brief 系统事件结构体
 * System Event Structure
 *
 * 通过 FreeRTOS 队列在各任务间传递。
 * Passed between tasks via FreeRTOS queues.
 */
typedef struct {
    sparkbot_event_type_t type;      /* 事件类型 | Event type */
    void                 *data;      /* 事件附加数据 | Event payload */
    int64_t               timestamp; /* 事件时间戳 (us) | Event timestamp */
} sparkbot_event_t;

/* ================================================================
 * 硬件配置结构体 | Hardware Configuration Structures
 * ================================================================ */

/**
 * @brief I2C 总线配置
 * I2C Bus Configuration
 */
typedef struct {
    i2c_port_t port;         /* I2C 端口号 (0 或 1) | I2C port number */
    uint8_t    scl_pin;      /* SCL 引脚 | SCL pin */
    uint8_t    sda_pin;      /* SDA 引脚 | SDA pin */
    uint32_t   frequency;    /* 时钟频率 (Hz), 默认 100kHz | Clock frequency, default 100kHz */
    uint8_t    clk_flags;    /* 内部上拉使能 | Internal pullup enable */
} sparkbot_i2c_config_t;

/**
 * @brief SPI 总线配置
 * SPI Bus Configuration
 */
typedef struct {
    spi_host_device_t host;  /* SPI 主机设备 | SPI host device */
    uint8_t           sclk;  /* 时钟引脚 | Clock pin */
    uint8_t           mosi;  /* MOSI 引脚 | MOSI pin */
    uint8_t           miso;  /* MISO 引脚 (可选) | MISO pin (optional) */
    int               max_transfer_sz; /* 最大传输大小 | Max transfer size */
} sparkbot_spi_config_t;

/* ================================================================
 * 公共 API | Public API
 * ================================================================ */

/**
 * @brief 初始化 SparkBot 核心系统
 * Initialize SparkBot Core System
 *
 * 注册所有硬件驱动和系统服务。
 * Registers all hardware drivers and system services.
 *
 * @param event_queue 事件队列句柄 | Event queue handle
 * @return ESP_OK 成功 | ESP_OK on success
 */
esp_err_t sparkbot_core_init(QueueHandle_t event_queue);

/**
 * @brief 反初始化 SparkBot 核心 (关机前清理)
 * De-initialize SparkBot Core (cleanup before shutdown)
 *
 * @return ESP_OK 成功 | ESP_OK on success
 */
esp_err_t sparkbot_core_deinit(void);

/* ================================================================
 * 硬件初始化 | Hardware Initialization
 * ================================================================ */

/**
 * @brief 初始化 I2C 总线 (IMU + 触摸屏共用)
 * Initialize I2C bus (shared by IMU + touch screen)
 *
 * @return ESP_OK 成功 | ESP_OK on success
 */
esp_err_t sparkbot_i2c_init(void);

/**
 * @brief 初始化 SPI 总线 (显示屏)
 * Initialize SPI bus (for display)
 *
 * @return ESP_OK 成功 | ESP_OK on success
 */
esp_err_t sparkbot_spi_init(void);

/**
 * @brief 初始化摄像头 (OV2640)
 * Initialize camera (OV2640)
 *
 * @return ESP_OK 成功 | ESP_OK on success
 */
esp_err_t sparkbot_camera_init(void);

/**
 * @brief 初始化音频系统 (I2S 输入/输出)
 * Initialize audio system (I2S input/output)
 *
 * @return ESP_OK 成功 | ESP_OK on success
 */
esp_err_t sparkbot_audio_init(void);

/**
 * @brief 初始化 WiFi 连接
 * Initialize WiFi connection
 *
 * @param event_group 事件组句柄 (用于通知连接状态)
 *                    Event group handle (for connection status notification)
 * @return ESP_OK 成功 | ESP_OK on success
 */
esp_err_t sparkbot_core_wifi_init(EventGroupHandle_t event_group);

/* ================================================================
 * 电源管理 | Power Management
 * ================================================================ */

/**
 * @brief 获取电池电压 (mV)
 * Get battery voltage in millivolts
 *
 * @return 电池电压 (mV), 如果 ADC 不可用返回 0
 *         Battery voltage in mV, 0 if ADC unavailable
 */
uint32_t sparkbot_core_get_battery_voltage(void);

/**
 * @brief 进入低功耗睡眠模式
 * Enter low-power sleep mode
 *
 * @param sleep_duration_ms 睡眠持续时间 (毫秒)
 *                          Sleep duration in milliseconds
 * @return ESP_OK 成功 | ESP_OK on success
 */
esp_err_t sparkbot_core_enter_sleep(uint32_t sleep_duration_ms);

/**
 * @brief 检测是否正在充电 (USB 连接检测)
 * Detect if charging (USB connection detection)
 *
 * @return true 正在充电 | Charging
 * @return false 未充电 | Not charging
 */
bool sparkbot_core_is_charging(void);

/* ================================================================
 * 系统信息 | System Information
 * ================================================================ */

/**
 * @brief 获取系统版本号
 * Get system version string
 *
 * @return 版本字符串 (如 "v1.0.0")
 */
const char *sparkbot_get_version(void);

/**
 * @brief 获取芯片温度 (摄氏度)
 * Get chip temperature in Celsius
 *
 * @return 温度值 | Temperature value
 */
float sparkbot_get_chip_temperature(void);

/**
 * @brief 获取 WiFi 信号强度 (dBm)
 * Get WiFi RSSI in dBm
 *
 * @return RSSI 值，连接失败返回 0
 */
int8_t sparkbot_get_wifi_rssi(void);

#ifdef __cplusplus
}
#endif

#endif /* SPARKBOT_CORE_H */
