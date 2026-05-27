/**
 * ================================================================
 * ESP-SparkBot -- 应用主入口
 * ESP-SparkBot -- Application Main Entry Point
 * ================================================================
 *
 * 替代原来的 src/main.py
 * Replaces the original src/main.py
 *
 * 功能 (Functions):
 *   - FreeRTOS 任务创建与调度  | FreeRTOS task creation & scheduling
 *   - 硬件初始化 (显示屏, 摄像头, 音频, BMI270, 触摸, WiFi)
 *     Hardware init (display, camera, audio, BMI270, touch, WiFi)
 *   - 主事件循环 | Main event loop
 *   - 任务优先级与栈空间管理 | Task priorities & stack sizing
 *
 * 芯片: ESP32-S3
 * 框架: ESP-IDF v5.3+
 */

/* ---- 标准头文件 (Standard Headers) ---- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

/* ---- ESP-IDF 系统头文件 (ESP-IDF System Headers) ---- */
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "esp_console.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

/* ---- 项目组件头文件 (Project Component Headers) ---- */
#include "sparkbot_core.h"
#include "lvgl_display.h"
#include "touch_motion.h"
#include "llm_dialog.h"
#include "state_machine.h"
#include "utils.h"

/* ---- 日志标签 (Log Tag) ---- */
static const char *TAG = "app_main";

/* ================================================================
 * 常量定义 | Constant Definitions
 * ================================================================ */

/* 事件组位定义 | Event Group Bit Definitions */
#define EVENT_WIFI_CONNECTED          BIT0  /* WiFi 连接成功 */
#define EVENT_WIFI_FAILED             BIT1  /* WiFi 连接失败 */
#define EVENT_NTP_SYNCED              BIT2  /* NTP 时间同步完成 */
#define EVENT_SYSTEM_READY            BIT3  /* 系统就绪 */
#define EVENT_SHUTDOWN                BIT4  /* 关机信号 */
#define EVENT_TOUCH_DETECTED          BIT5  /* 触摸检测 */
#define EVENT_GESTURE_DETECTED        BIT6  /* 手势检测 */
#define EVENT_BUTTON_PRESSED          BIT7  /* 按键按下 */

/* ================================================================
 * 任务优先级定义 | Task Priority Definitions
 * ================================================================
 *
 * FreeRTOS 任务优先级: 数字越大优先级越高 (0 是最低)
 * ESP-IDF 默认空闲任务优先级: 0
 * ESP-IDF 系统事件任务优先级: 20
 * 用户任务建议范围: 1-19
 */

/* 音频处理优先级最高 — 需要实时性 */
/* Audio processing gets highest priority — needs real-time */
#define TASK_PRIORITY_AUDIO_PROCESS    (configMAX_PRIORITIES - 2)  /* 23 */

/* 显示刷新优先级 — 60FPS 要求 */
/* Display refresh priority — 60FPS requirement */
#define TASK_PRIORITY_DISPLAY          (configMAX_PRIORITIES - 3)  /* 22 */

/* AI 对话处理 — 可能阻塞在HTTP请求 */
/* AI dialogue processing — may block on HTTP requests */
#define TASK_PRIORITY_AI_DIALOG        (configMAX_PRIORITIES - 5)  /* 20 */

/* 传感器轮询 — IMU + 触摸 */
/* Sensor polling — IMU + touch */
#define TASK_PRIORITY_SENSOR           (configMAX_PRIORITIES - 6)  /* 19 */

/* 状态机逻辑 — 低优先级 */
/* State machine logic — low priority */
#define TASK_PRIORITY_STATE_MACHINE    (configMAX_PRIORITIES - 7)  /* 18 */

/* 后台维护任务 — 最低优先级 */
/* Background maintenance — lowest priority */
#define TASK_PRIORITY_MAINTENANCE      (configMAX_PRIORITIES - 8)  /* 17 */

/* ================================================================
 * 任务栈大小定义 | Task Stack Size Definitions
 * ================================================================
 *
 * 栈大小单位: 字 (word = 4 bytes on ESP32-S3)
 * 经验值: 基础开销 ~2KB, 每层函数调用 ~256B
 */

/* 显示任务需要操作 LVGL 对象，栈需求较大 */
/* Display task manipulates LVGL objects, needs larger stack */
#define STACK_SIZE_DISPLAY             (6 * 1024)

/* AI 对话任务需要 HTTP 客户端 + JSON 解析 */
/* AI dialogue task needs HTTP client + JSON parsing */
#define STACK_SIZE_AI_DIALOG           (10 * 1024)

/* 音频处理任务 — WAV 解码 + I2S DMA */
/* Audio processing task — WAV decode + I2S DMA */
#define STACK_SIZE_AUDIO               (8 * 1024)

/* 传感器任务 — I2C 通信 + 数据滤波 */
/* Sensor task — I2C comm + data filtering */
#define STACK_SIZE_SENSOR              (4 * 1024)

/* 状态机任务 — 纯逻辑，栈需求小 */
/* State machine task — pure logic, small stack */
#define STACK_SIZE_STATE_MACHINE       (3 * 1024)

/* 维护任务 */
#define STACK_SIZE_MAINTENANCE         (3 * 1024)

/* ================================================================
 * 全局变量 | Global Variables
 * ================================================================ */

/* ---- 事件组 (Event Group) ---- */
/* 用于跨任务通信的事件组句柄 */
/* Event group handle for cross-task communication */
static EventGroupHandle_t s_event_group = NULL;

/* ---- 任务句柄 (Task Handles) ---- */
/* 保存各任务句柄以便外部控制（挂起/恢复/删除） */
/* Store task handles for external control (suspend/resume/delete) */
static TaskHandle_t s_task_display       = NULL;
static TaskHandle_t s_task_ai_dialog     = NULL;
static TaskHandle_t s_task_audio         = NULL;
static TaskHandle_t s_task_sensor        = NULL;
static TaskHandle_t s_task_state_machine = NULL;

/* ---- 队列 (Queues) ---- */
/* 事件队列: 用于向主循环传递事件 */
/* Event queue: passes events to the main loop */
static QueueHandle_t s_event_queue = NULL;

/* ---- 系统状态 (System State) ---- */
static sparkbot_state_t s_system_state = SPARKBOT_STATE_INIT;

/* ================================================================
 * 前向声明 | Forward Declarations
 * ================================================================ */
static void init_nvs(void);
static void init_network(void);
static void init_hardware(void);
static void task_display(void *pvParameters);
static void task_ai_dialog(void *pvParameters);
static void task_audio_process(void *pvParameters);
static void task_sensor_poll(void *pvParameters);
static void task_state_machine(void *pvParameters);
static void task_maintenance(void *pvParameters);
static void event_handler_sparkbot(sparkbot_event_t event, void *data);

/* ================================================================
 * 主体 | Main Body
 * ================================================================ */

/**
 * @brief ESP-IDF 应用入口函数
 * ESP-IDF Application Entry Point
 *
 * 这是整个固件的起点，由 ESP-IDF 启动代码调用。
 * This is the starting point of the entire firmware, called by ESP-IDF boot code.
 *
 * 启动流程 | Boot Sequence:
 *   1. 初始化 NVS (非易失性存储) | Init NVS
 *   2. 初始化网络 (WiFi + NTP)   | Init network (WiFi + NTP)
 *   3. 初始化硬件外设             | Init hardware peripherals
 *   4. 创建 FreeRTOS 任务         | Create FreeRTOS tasks
 *   5. 进入主事件循环             | Enter main event loop
 */
void app_main(void)
{
    sparkbot_event_t event;

    ESP_LOGI(TAG, "╔══════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║    ESP-SparkBot 启动中... v1.0.0       ║");
    ESP_LOGI(TAG, "║    ESP-SparkBot Booting...              ║");
    ESP_LOGI(TAG, "╚══════════════════════════════════════════╝");

    /* ---- 步骤1: 初始化 NVS (Step 1: Init NVS) ----
     * NVS 存储 WiFi 凭证、AI Token 等持久化配置
     * NVS stores WiFi credentials, AI tokens, and other persistent config
     */
    init_nvs();

    /* ---- 步骤2: 初始化事件系统 (Step 2: Init Event System) ---- */
    s_event_group = xEventGroupCreate();
    if (s_event_group == NULL) {
        ESP_LOGE(TAG, "[错误] 创建事件组失败 | Failed to create event group");
        return;
    }

    /* 事件队列: 最多缓存 32 个事件 */
    /* Event queue: max 32 buffered events */
    s_event_queue = xQueueCreate(32, sizeof(sparkbot_event_t));
    if (s_event_queue == NULL) {
        ESP_LOGE(TAG, "[错误] 创建事件队列失败 | Failed to create event queue");
        return;
    }

    /* ---- 步骤3: 初始化网络 (Step 3: Init Network) ---- */
    init_network();

    /* 等待 WiFi 连接 (最多等待 30 秒) */
    /* Wait for WiFi connection (max 30 seconds) */
    EventBits_t bits = xEventGroupWaitBits(
        s_event_group,
        EVENT_WIFI_CONNECTED | EVENT_WIFI_FAILED,
        pdFALSE,       /* 不清除位 | Don't clear bits */
        pdFALSE,       /* 任一位置位即返回 | Return if any bit is set */
        pdMS_TO_TICKS(30000)
    );

    if (bits & EVENT_WIFI_FAILED) {
        ESP_LOGW(TAG, "[警告] WiFi 连接失败，部分功能不可用 | WiFi failed, some features unavailable");
    }

    /* ---- 步骤4: 初始化硬件 (Step 4: Init Hardware) ---- */
    init_hardware();

    /* ---- 步骤5: 创建 FreeRTOS 任务 (Step 5: Create FreeRTOS Tasks) ---- */

    /* 显示任务 | Display Task */
    xTaskCreatePinnedToCore(
        task_display,           /* 任务函数 | Task function */
        "display",              /* 任务名称 | Task name */
        STACK_SIZE_DISPLAY,     /* 栈大小 (bytes) */
        NULL,                   /* 参数 | Parameter */
        TASK_PRIORITY_DISPLAY,  /* 优先级 */
        &s_task_display,        /* 任务句柄 | Task handle */
        1                       /* 固定到核心1 | Pin to core 1 */
    );

    /* AI 对话任务 |
    /* AI dialog task */
    xTaskCreatePinnedToCore(
        task_ai_dialog,
        "ai_dialog",
        STACK_SIZE_AI_DIALOG,
        NULL,
        TASK_PRIORITY_AI_DIALOG,
        &s_task_ai_dialog,
        1                       /* Core 1 — HTTP 请求在此核心处理 */
    );

    /* 音频处理任务 | Audio processing task */
    xTaskCreatePinnedToCore(
        task_audio_process,
        "audio",
        STACK_SIZE_AUDIO,
        NULL,
        TASK_PRIORITY_AUDIO_PROCESS,
        &s_task_audio,
        0                       /* Core 0 — 靠近 I2S 硬件中断 */
    );

    /* 传感器轮询任务 | Sensor polling task */
    xTaskCreatePinnedToCore(
        task_sensor_poll,
        "sensor",
        STACK_SIZE_SENSOR,
        NULL,
        TASK_PRIORITY_SENSOR,
        &s_task_sensor,
        0                       /* Core 0 */
    );

    /* 状态机任务 | State machine task */
    xTaskCreatePinnedToCore(
        task_state_machine,
        "state_machine",
        STACK_SIZE_STATE_MACHINE,
        NULL,
        TASK_PRIORITY_STATE_MACHINE,
        &s_task_state_machine,
        0                       /* Core 0 */
    );

    /* 后台维护任务 | Background maintenance task */
    xTaskCreatePinnedToCore(
        task_maintenance,
        "maintenance",
        STACK_SIZE_MAINTENANCE,
        NULL,
        TASK_PRIORITY_MAINTENANCE,
        NULL,
        0                       /* Core 0 */
    );

    /* ---- 步骤6: 系统就绪 (Step 6: System Ready) ---- */
    xEventGroupSetBits(s_event_group, EVENT_SYSTEM_READY);
    ESP_LOGI(TAG, "[系统] 全部任务已启动 | All tasks started");
    ESP_LOGI(TAG, "[提示] 触摸屏操作 | Touch to interact");
    ESP_LOGI(TAG, "[提示] 双击开启AI对话 | Double-tap for AI chat");

    /* ============================================================
     * 主事件循环 | Main Event Loop
     * ============================================================
     *
     * 主循环从事件队列中读取事件并分发到对应处理函数。
     * 这是基于事件的架构 —— 所有行为都由事件驱动。
     *
     * The main loop reads events from the event queue and dispatches
     * them to the appropriate handler. This is an event-driven
     * architecture — all behavior is driven by events.
     */
    while (1) {
        /* 阻塞等待事件 (无限超时) */
        /* Block waiting for event (infinite timeout) */
        if (xQueueReceive(s_event_queue, &event, portMAX_DELAY) == pdTRUE) {
            ESP_LOGD(TAG, "[事件] 收到事件 type=%d | Received event type=%d",
                     event.type);

            event_handler_sparkbot(event, NULL);

            /* 检查关机事件 */
            /* Check shutdown event */
            if (event.type == SPARKBOT_EVENT_SHUTDOWN) {
                ESP_LOGI(TAG, "[系统] 收到关机指令，正在关机... | Shutdown requested...");
                break;
            }
        }
    }

    /* ---- 清理 (Cleanup) ---- */
    /* ESP-IDF 的 main 函数返回后会自动重启 */
    /* ESP-IDF auto-restarts when main() returns */
    ESP_LOGI(TAG, "[系统] 主循环退出 | Main loop exited");
}

/* ================================================================
 * 初始化函数 | Initialization Functions
 * ================================================================ */

/**
 * @brief 初始化 NVS (非易失性存储)
 * Init Non-Volatile Storage
 *
 * NVS 用于存储 WiFi 凭证、设备配置等需要断电保持的数据。
 * NVS is used to store WiFi credentials, device config, and other
 * data that must persist across power cycles.
 */
static void init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /* NVS 分区被截断或版本不匹配 — 擦除并重试 */
        /* NVS partition truncated or version mismatch — erase & retry */
        ESP_LOGW(TAG, "[NVS] 擦除并重新初始化 | Erasing & reinitializing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "[NVS] 初始化完成 | Initialized OK");
}

/**
 * @brief 初始化网络 (WiFi + TCP/IP)
 * Init Network (WiFi + TCP/IP)
 *
 * ESP-IDF 使用 esp_netif 抽象层管理网络接口。
 * ESP-IDF uses the esp_netif abstraction layer to manage network interfaces.
 */
static void init_network(void)
{
    /* 初始化 TCP/IP 协议栈 */
    /* Init TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_init());

    /* 创建默认事件循环 */
    /* Create default event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* 创建默认 WiFi 站点接口 */
    /* Create default WiFi station interface */
    esp_netif_create_default_wifi_sta();

    /* 初始化 WiFi 驱动 */
    /* Initialize WiFi driver */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* 注册 WiFi 事件处理 */
    /* Register WiFi event handler */
    sparkbot_core_wifi_init(s_event_group);

    ESP_LOGI(TAG, "[网络] WiFi 初始化完成 | WiFi initialized");
    ESP_LOGI(TAG, "[网络] 正在连接 WiFi... | Connecting to WiFi...");
}

/**
 * @brief 初始化所有硬件外设
 * Initialize All Hardware Peripherals
 *
 * 初始化流程说明:
 * Init sequence notes:
 *   - I2C 总线必须先初始化 (IMU, 触摸屏共用)
 *     I2C bus must be init'd first (shared by IMU, touch)
 *   - SPI 总线用于显示屏
 *     SPI bus used for display
 *   - 摄像头使用 8-bit 并行接口
 *     Camera uses 8-bit parallel interface
 *   - I2S 用于音频输入和输出
 *     I2S used for both audio input and output
 */
static void init_hardware(void)
{
    ESP_LOGI(TAG, "[硬件] 开始初始化外设... | Starting hardware init...");

    /* 1. I2C 总线初始化 (IMU + 触摸屏共用) */
    /* 1. I2C bus init (shared by IMU + touch) */
    ESP_ERROR_CHECK(sparkbot_i2c_init());

    /* 2. SPI 总线初始化 (显示屏) */
    /* 2. SPI bus init (display) */
    ESP_ERROR_CHECK(sparkbot_spi_init());

    /* 3. 显示屏初始化 (GC9A01 240x240 圆形屏) */
    /* 3. Display init (GC9A01 240x240 round display) */
    ESP_ERROR_CHECK(lvgl_display_init());

    /* 4. 摄像头初始化 (OV2640) */
    /* 4. Camera init (OV2640) */
#if CONFIG_SPARKBOT_CAMERA_ENABLE
    ESP_ERROR_CHECK(sparkbot_camera_init());
#endif

    /* 5. 音频初始化 (I2S 输出 + 麦克风输入) */
    /* 5. Audio init (I2S output + mic input) */
#if CONFIG_SPARKBOT_AUDIO_ENABLE
    ESP_ERROR_CHECK(sparkbot_audio_init());
#endif

    /* 6. IMU 初始化 (BMI270) */
    /* 6. IMU init (BMI270) */
#if CONFIG_SPARKBOT_IMU_ENABLE
    ESP_ERROR_CHECK(touch_motion_imu_init());
#endif

    /* 7. 触摸屏初始化 (CST816T) */
    /* 7. Touch panel init (CST816T) */
#if CONFIG_SPARKBOT_TOUCH_ENABLE
    ESP_ERROR_CHECK(touch_motion_touch_init());
#endif

    /* 8. 按键初始化 */
    /* 8. Button init */
#if CONFIG_SPARKBOT_BUTTON_ENABLE
    gpio_config_t btn_cfg = {
        .pin_bit_mask = BIT64(CONFIG_SPARKBOT_BUTTON_PIN_BOOT),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&btn_cfg);
#endif

    /* 9. 初始化核心系统 */
    /* 9. Initialize core system */
    sparkbot_core_init(s_event_queue);

    ESP_LOGI(TAG, "[硬件] 所有外设初始化完成 | All peripherals initialized");
}

/* ================================================================
 * FreeRTOS 任务函数 | FreeRTOS Task Functions
 * ================================================================ */

/**
 * @brief 显示任务
 * Display Task
 *
 * 负责:
 *   - 驱动 LVGL 帧渲染
 *   - 动画帧切换
 *   - UI 元素更新
 *   - 表情切换
 *
 * Responsibilities:
 *   - Drive LVGL frame rendering
 *   - Animation frame switching
 *   - UI element updates
 *   - Expression switching
 *
 * 帧率: 目标 60 FPS (16ms 一帧)
 * Frame rate: target 60 FPS (16ms per frame)
 *
 * 核心: 固定在 Core 1，避免与 WiFi 中断冲突
 * Core: pinned to Core 1 to avoid WiFi interrupt conflicts
 */
static void task_display(void *pvParameters)
{
    /* 等待系统就绪 */
    /* Wait for system ready */
    xEventGroupWaitBits(s_event_group, EVENT_SYSTEM_READY,
                        pdFALSE, pdTRUE, portMAX_DELAY);

    ESP_LOGI(TAG, "[显示] 显示任务启动 | Display task started");

    /* 显示启动画面 */
    lvgl_display_show_boot_logo();

    const TickType_t period = pdMS_TO_TICKS(16);  /* ~60 FPS */
    TickType_t last_wake = xTaskGetTickCount();

    while (1) {
        /* LVGL 核心定时器处理 (必须周期性调用) */
        /* LVGL core tick handler (must be called periodically) */
        lv_tick_inc(16);

        /* LVGL 任务处理 — 刷新屏幕 */
        /* LVGL task handler — refresh screen */
        lv_task_handler();

        /* 更新动画帧 */
        /* Update animation frame */
        lvgl_display_update_animation();

        /* 精确延时到下一帧 */
        /* Precise delay to next frame */
        vTaskDelayUntil(&last_wake, period);
    }
}

/**
 * @brief AI 对话任务
 * AI Dialogue Task
 *
 * 负责:
 *   - HTTP 请求发送到 LLM API
 *   - 流式响应接收与解析
 *   - 对话历史管理
 *   - TTS 请求触发
 *
 * Responsibilities:
 *   - Send HTTP requests to LLM API
 *   - Receive and parse streaming response
 *   - Conversation history management
 *   - Trigger TTS requests
 *
 * 高栈需求: HTTP 客户端 + JSON 解析 + TLS 握手的嵌套调用
 * High stack requirement: nested calls for HTTP client + JSON + TLS
 */
static void task_ai_dialog(void *pvParameters)
{
    /* 等待系统就绪 */
    /* Wait for system ready */
    xEventGroupWaitBits(s_event_group, EVENT_SYSTEM_READY,
                        pdFALSE, pdTRUE, portMAX_DELAY);

    ESP_LOGI(TAG, "[AI对话] AI对话任务启动 | AI dialog task started");

    /* 初始化 AI 对话管理器 */
    /* Initialize AI dialog manager */
    llm_dialog_init();

    sparkbot_event_t event;

    while (1) {
        /* 阻塞等待 AI 对话事件 */
        /* Block waiting for AI dialog events */
        if (xQueueReceive(s_event_queue, &event, pdMS_TO_TICKS(1000)) == pdTRUE) {
            switch (event.type) {
                case SPARKBOT_EVENT_AI_CHAT_REQUEST: {
                    /* 用户发起了 AI 对话请求 */
                    /* User initiated AI chat request */
                    const char *user_msg = (const char *)event.data;
                    ESP_LOGI(TAG, "[AI对话] 用户: %s | User: %s", user_msg);

                    /* 调用 LLM API 获取回复 */
                    /* Call LLM API to get reply */
                    char *ai_response = llm_dialog_chat(user_msg);
                    if (ai_response != NULL) {
                        ESP_LOGI(TAG, "[AI对话] AI: %s", ai_response);

                        /* 触发 TTS 语音播放 */
                        /* Trigger TTS playback */
                        sparkbot_event_t tts_event = {
                            .type = SPARKBOT_EVENT_TTS_PLAY,
                            .data = ai_response,
                            .timestamp = esp_timer_get_time(),
                        };
                        xQueueSend(s_event_queue, &tts_event, pdMS_TO_TICKS(100));

                        free(ai_response);
                    } else {
                        ESP_LOGE(TAG, "[AI对话] 获取AI回复失败 | Failed to get AI response");
                    }
                    break;
                }

                case SPARKBOT_EVENT_AI_CLEAR_HISTORY:
                    /* 清除对话历史 */
                    /* Clear conversation history */
                    llm_dialog_clear_history();
                    ESP_LOGI(TAG, "[AI对话] 对话历史已清除 | History cleared");
                    break;

                default:
                    break;
            }
        }
    }
}

/**
 * @brief 音频处理任务
 * Audio Processing Task
 *
 * 负责:
 *   - 麦克风录音 (I2S 输入)
 *   - 语音活动检测 (VAD)
 *   - TTS 音频播放 (I2S 输出)
 *
 * Responsibilities:
 *   - Microphone recording (I2S input)
 *   - Voice activity detection (VAD)
 *   - TTS audio playback (I2S output)
 */
static void task_audio_process(void *pvParameters)
{
    xEventGroupWaitBits(s_event_group, EVENT_SYSTEM_READY,
                        pdFALSE, pdTRUE, portMAX_DELAY);

    ESP_LOGI(TAG, "[音频] 音频处理任务启动 | Audio task started");

    sparkbot_event_t event;

    while (1) {
        if (xQueueReceive(s_event_queue, &event, pdMS_TO_TICKS(100)) == pdTRUE) {
            switch (event.type) {
                case SPARKBOT_EVENT_START_LISTENING:
                    /* 开始语音识别 | Start speech recognition */
                    ESP_LOGI(TAG, "[音频] 开始聆听... | Listening...");
                    /* TODO: 启动 I2S 录音 + VAD 检测 */
                    break;

                case SPARKBOT_EVENT_TTS_PLAY: {
                    /* 播放 TTS 音频 | Play TTS audio */
                    const char *text = (const char *)event.data;
                    if (text != NULL) {
                        ESP_LOGI(TAG, "[音频] TTS播放: %s | Playing TTS: %s", text);
                        /* TODO: 调用百度TTS API → 获取音频 → I2S播放 */
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }
}

/**
 * @brief 传感器轮询任务
 * Sensor Polling Task
 *
 * 负责:
 *   - BMI270 IMU 数据读取 (加速度+陀螺仪)
 *   - 手势识别 (双击, 摇晃, 翻转等)
 *   - 电容触摸屏检测
 *   - 物理按键扫描
 *
 * Responsibilities:
 *   - BMI270 IMU data reading (accel + gyro)
 *   - Gesture recognition (double-tap, shake, flip, etc.)
 *   - Capacitive touch detection
 *   - Physical button scanning
 *
 * 轮询周期: 10ms (100Hz) — 满足手势识别需求
 * Polling period: 10ms (100Hz) — meets gesture recognition needs
 */
static void task_sensor_poll(void *pvParameters)
{
    xEventGroupWaitBits(s_event_group, EVENT_SYSTEM_READY,
                        pdFALSE, pdTRUE, portMAX_DELAY);

    ESP_LOGI(TAG, "[传感器] 传感器轮询任务启动 | Sensor poll task started");

    touch_data_t touch_data;
    imu_data_t   imu_data;
    uint8_t      gesture;
    TickType_t   last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(10);  /* 100Hz */

    while (1) {
        /* ---- 读取触摸数据 | Read touch data ---- */
        if (touch_motion_read_touch(&touch_data) == ESP_OK) {
            if (touch_data.touched) {
                /* 检测到触摸 → 发送事件 */
                /* Touch detected → send event */
                sparkbot_event_t event = {
                    .type      = SPARKBOT_EVENT_TOUCH,
                    .data      = &touch_data,
                    .timestamp = esp_timer_get_time(),
                };
                xQueueSend(s_event_queue, &event, 0);
            }
        }

        /* ---- 读取 IMU 数据 | Read IMU data ---- */
        if (touch_motion_read_imu(&imu_data) == ESP_OK) {
            /* 手势识别 | Gesture recognition */
            gesture = touch_motion_detect_gesture(&imu_data);
            if (gesture != GESTURE_NONE) {
                sparkbot_event_t event = {
                    .type      = SPARKBOT_EVENT_GESTURE,
                    .data      = &gesture,
                    .timestamp = esp_timer_get_time(),
                };
                xQueueSend(s_event_queue, &event, 0);
            }
        }

        /* ---- 读取按键 | Read button ---- */
#if CONFIG_SPARKBOT_BUTTON_ENABLE
        static uint8_t btn_last = 1;
        uint8_t btn_now = gpio_get_level(CONFIG_SPARKBOT_BUTTON_PIN_BOOT);
        if (btn_last == 1 && btn_now == 0) {
            /* 按键按下 (下降沿) */
            /* Button pressed (falling edge) */
            sparkbot_event_t event = {
                .type      = SPARKBOT_EVENT_BUTTON,
                .data      = NULL,
                .timestamp = esp_timer_get_time(),
            };
            xQueueSend(s_event_queue, &event, 0);
        }
        btn_last = btn_now;
#endif

        /* 精确延时 | Precise delay */
        vTaskDelayUntil(&last_wake, period);
    }
}

/**
 * @brief 状态机任务
 * State Machine Task
 *
 * 负责:
 *   - 宠物行为状态管理 (IDLE, LISTENING, SPEAKING, GAMING, CHARGING, ERROR)
 *   - 状态切换逻辑
 *   - 需要系统响应
 *
 * Responsibilities:
 *   - Pet behavior state management
 *   - State transition logic
 *   - Needs system response
 */
static void task_state_machine(void *pvParameters)
{
    xEventGroupWaitBits(s_event_group, EVENT_SYSTEM_READY,
                        pdFALSE, pdTRUE, portMAX_DELAY);

    ESP_LOGI(TAG, "[状态机] 状态机任务启动 | State machine task started");

    /* 初始化状态机 */
    sm_init(s_event_queue);

    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(50);  /* 20Hz — 足够行为更新 */

    while (1) {
        /* 状态机主更新循环 */
        /* State machine main update loop */
        sm_update();

        vTaskDelayUntil(&last_wake, period);
    }
}

/**
 * @brief 后台维护任务
 * Background Maintenance Task
 *
 * 负责:
 *   - 电池电压监测
 *   - 空闲睡眠管理
 *   - 系统状态 LED 控制
 *   - 日志输出
 *   - 看门狗喂狗
 *
 * Responsibilities:
 *   - Battery voltage monitoring
 *   - Idle sleep management
 *   - System status LED control
 *   - Log output
 *   - Watchdog feeding
 */
static void task_maintenance(void *pvParameters)
{
    xEventGroupWaitBits(s_event_group, EVENT_SYSTEM_READY,
                        pdFALSE, pdTRUE, portMAX_DELAY);

    ESP_LOGI(TAG, "[维护] 维护任务启动 | Maintenance task started");

    /* 空闲计时器 | Idle timer */
    uint32_t idle_ms = 0;
    sparkbot_state_t last_state = SPARKBOT_STATE_IDLE;

    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(1000);  /* 1Hz */

    while (1) {
        /* ---- 电池监测 | Battery monitor ---- */
#if CONFIG_SPARKBOT_POWER_MANAGEMENT_ENABLE
        uint32_t battery_mv = sparkbot_core_get_battery_voltage();
        if (battery_mv < CONFIG_SPARKBOT_BATTERY_LOW_THRESHOLD) {
            ESP_LOGW(TAG, "[电池] 电量低! %lumV | Low battery!", battery_mv);
            /* 触发低电量事件 | Trigger low battery event */
            sparkbot_event_t event = {
                .type      = SPARKBOT_EVENT_LOW_BATTERY,
                .data      = &battery_mv,
                .timestamp = esp_timer_get_time(),
            };
            xQueueSend(s_event_queue, &event, 0);
        }
#endif

        /* ---- 空闲超时检查 | Idle timeout check ---- */
        sparkbot_state_t current_state = sm_get_state();
        if (current_state == SPARKBOT_STATE_IDLE) {
            idle_ms += 1000;
#if CONFIG_SPARKBOT_SLEEP_TIMEOUT
            if (idle_ms >= CONFIG_SPARKBOT_SLEEP_TIMEOUT) {
                ESP_LOGI(TAG, "[维护] 进入睡眠模式 | Entering sleep mode");
                sm_request_transition(SPARKBOT_STATE_SLEEP);
                idle_ms = 0;
            }
#endif
        } else {
            idle_ms = 0;
        }
        last_state = current_state;

        /* ---- 堆内存使用报告 (每60秒) | Heap usage report (every 60s) ---- */
        static uint8_t report_counter = 0;
        if (++report_counter >= 60) {
            report_counter = 0;
            size_t free_heap = esp_get_free_heap_size();
            size_t min_free  = esp_get_minimum_free_heap_size();
            ESP_LOGI(TAG, "[内存] 空闲: %u B, 最小空闲: %u B | Free: %u B, Min free: %u B",
                     free_heap, min_free);
        }

        vTaskDelayUntil(&last_wake, period);
    }
}

/* ================================================================
 * 事件分发器 | Event Dispatcher
 * ================================================================ */

/**
 * @brief SparkBot 全局事件处理
 * SparkBot Global Event Handler
 *
 * 根据事件类型分发到对应的处理子系统。
 * Dispatches to the appropriate subsystem based on event type.
 *
 * @param event 事件结构体 | Event struct
 * @param data  附加数据 (未使用) | Additional data (unused)
 */
static void event_handler_sparkbot(sparkbot_event_t event, void *data)
{
    (void)data;  /* 未使用 | Unused */

    switch (event.type) {
        /* ---- 触摸事件 | Touch Events ---- */
        case SPARKBOT_EVENT_TOUCH: {
            touch_data_t *touch = (touch_data_t *)event.data;
            if (touch != NULL) {
                ESP_LOGD(TAG, "[触摸] x=%d y=%d | Touch x=%d y=%d",
                         touch->x, touch->y);
                /* 通知状态机用户互动 */
                /* Notify state machine of user interaction */
                sm_handle_event(SM_EVENT_TOUCH, touch);
            }
            break;
        }

        /* ---- 手势事件 | Gesture Events ---- */
        case SPARKBOT_EVENT_GESTURE: {
            uint8_t *gesture = (uint8_t *)event.data;
            if (gesture != NULL) {
                ESP_LOGI(TAG, "[手势] 检测到手势: %d | Gesture detected: %d", *gesture);
                switch (*gesture) {
                    case GESTURE_DOUBLE_TAP:
                        /* 双击 → 开启 AI 对话 */
                        /* Double tap → Start AI dialogue */
                        sm_handle_event(SM_EVENT_DOUBLE_TAP, NULL);
                        break;
                    case GESTURE_SHAKE:
                        /* 摇晃 → 切换表情 */
                        /* Shake → Switch expression */
                        sm_handle_event(SM_EVENT_SHAKE, NULL);
                        break;
                    case GESTURE_TILT_LEFT:
                    case GESTURE_TILT_RIGHT:
                        /* 倾斜 → 触发小游戏 */
                        /* Tilt → Trigger mini-game */
                        sm_handle_event(SM_EVENT_TILT, gesture);
                        break;
                    default:
                        break;
                }
            }
            break;
        }

        /* ---- 按键事件 | Button Events ---- */
        case SPARKBOT_EVENT_BUTTON:
            ESP_LOGI(TAG, "[按键] 按键按下 | Button pressed");
            sm_handle_event(SM_EVENT_BUTTON_PRESS, NULL);
            break;

        /* ---- AI 对话结果 | AI Chat Result ---- */
        case SPARKBOT_EVENT_AI_CHAT_RESPONSE: {
            const char *response = (const char *)event.data;
            if (response != NULL) {
                ESP_LOGI(TAG, "[AI] %s", response);
                lvgl_display_show_text_bubble(response, 5000);
            }
            break;
        }

        /* ---- 低电量 | Low Battery ---- */
        case SPARKBOT_EVENT_LOW_BATTERY: {
            uint32_t *mv = (uint32_t *)event.data;
            ESP_LOGW(TAG, "[电池] 低电量警报: %lumV | Low battery alert!", mv ? *mv : 0);
            lvgl_display_show_warning("电量不足! 请充电");
            break;
        }

        /* ---- 错误事件 | Error Event ---- */
        case SPARKBOT_EVENT_ERROR: {
            ESP_LOGE(TAG, "[错误] 系统错误 | System error");
            sm_force_state(SPARKBOT_STATE_ERROR);
            break;
        }

        /* ---- 关机事件 | Shutdown ---- */
        case SPARKBOT_EVENT_SHUTDOWN:
            ESP_LOGI(TAG, "[系统] 关机中... | Shutting down...");
            sparkbot_core_deinit();
            break;

        default:
            ESP_LOGW(TAG, "[事件] 未知事件类型: %d | Unknown event type: %d",
                     event.type);
            break;
    }
}
