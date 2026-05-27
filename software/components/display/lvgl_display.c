/**
 * ================================================================
 * ESP-SparkBot -- 显示管理实现 (LVGL)
 * ESP-SparkBot -- Display Manager Implementation (LVGL)
 * ================================================================
 *
 * 替代原来的 src/animation.py
 * Replaces the original src/animation.py
 *
 * GC9A01 圆形显示屏驱动 + LVGL UI 系统。
 * GC9A01 round display driver + LVGL UI system.
 *
 * 硬件连接 (Hardware Connections):
 *   - SCLK: GPIO 12
 *   - MOSI: GPIO 11
 *   - DC:   GPIO 10
 *   - CS:   GPIO 9
 *   - RST:  GPIO 8
 *   - BL:   GPIO 45 (PWM 背光控制)
 *
 * GC9A01 是一个 240x240 圆形 TFT LCD 控制器，
 * 通过 4 线 SPI 接口通信。RGB565 色深 (65K 色)。
 * GC9A01 is a 240x240 round TFT LCD controller,
 * communicating over 4-wire SPI. RGB565 color depth (65K colors).
 */

#include "lvgl_display.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include <string.h>

static const char *TAG = "display";

/* ================================================================
 * 全局变量 | Global Variables
 * ================================================================ */

/* LVGL 显示缓冲区 | LVGL display buffers */
static lv_disp_draw_buf_t s_draw_buf;
static lv_color_t        *s_buf1 = NULL;
static lv_color_t        *s_buf2 = NULL;

/* LVGL 显示驱动 | LVGL display driver */
static lv_disp_drv_t  s_disp_drv;
static lv_disp_t      *s_disp = NULL;

/* LCD 面板句柄 | LCD panel handle */
static esp_lcd_panel_io_handle_t s_io_handle = NULL;
static esp_lcd_panel_handle_t    s_panel_handle = NULL;

/* UI 对象 | UI Objects */
static lv_obj_t *s_scr_main    = NULL;  /* 主屏幕 | Main screen */
static lv_obj_t *s_label_face  = NULL;  /* 表情文字标签 | Expression text label */
static lv_obj_t *s_label_bubble = NULL; /* 气泡文字标签 | Bubble text label */
static lv_obj_t *s_label_warning = NULL;/* 警告文字标签 | Warning text label */
static lv_obj_t *s_bar_battery = NULL;  /* 电池电量条 | Battery bar */
static lv_obj_t *s_label_wifi  = NULL;  /* WiFi 图标 | WiFi icon */

/* 状态变量 | State Variables */
static sparkbot_expression_t s_current_expr = EXPR_NORMAL;
static uint8_t  s_brightness = 80;
static bool     s_sleeping = false;
static int64_t  s_bubble_show_time = 0;
static uint32_t s_bubble_duration = 5000;
static bool     s_bubble_visible = false;

/* ================================================================
 * 表情字符串映射 | Expression String Mapping
 * ================================================================ */

/**
 * @brief 表情文字映射表
 * Expression text mapping table
 *
 * 每个表情对应一个可爱的 ASCII 表情文字。
 * Each expression maps to a cute ASCII emoji text.
 */
static const char *s_expr_strings[EXPR_COUNT] = {
    [EXPR_NORMAL]    = "^_^",
    [EXPR_HAPPY]     = "≧▽≦",
    [EXPR_SAD]       = "╥_╥",
    [EXPR_SURPRISED] = "○_○",
    [EXPR_ANGRY]     = "-_-#",
    [EXPR_SLEEPY]    = "－_－\nzZZ",
    [EXPR_LOVE]      = " ♥ \n♥ ♥",
    [EXPR_LISTENING] = "◎_◎?",
    [EXPR_SPEAKING]  = "●_●!",
    [EXPR_THINKING]  = "...?",
    [EXPR_CHARGING]  = "⚡_⚡",
    [EXPR_ERROR]     = "╳_╳!",
};

/* 表情颜色 (RGB565) | Expression colors (RGB565) */
static const lv_color_t s_expr_colors[EXPR_COUNT] = {
    [EXPR_NORMAL]    = LV_COLOR_MAKE(100, 200, 100), /* 绿色 | Green */
    [EXPR_HAPPY]     = LV_COLOR_MAKE(255, 200, 50),  /* 金黄 | Gold */
    [EXPR_SAD]       = LV_COLOR_MAKE(100, 150, 220), /* 蓝色 | Blue */
    [EXPR_SURPRISED] = LV_COLOR_MAKE(255, 150, 50),  /* 橙色 | Orange */
    [EXPR_ANGRY]     = LV_COLOR_MAKE(255, 80,  80),  /* 红色 | Red */
    [EXPR_SLEEPY]    = LV_COLOR_MAKE(150, 150, 200), /* 淡紫 | Light purple */
    [EXPR_LOVE]      = LV_COLOR_MAKE(255, 100, 150), /* 粉红 | Pink */
    [EXPR_LISTENING] = LV_COLOR_MAKE(80,  200, 240), /* 青色 | Cyan */
    [EXPR_SPEAKING]  = LV_COLOR_MAKE(200, 120, 50),  /* 棕色 | Brown */
    [EXPR_THINKING]  = LV_COLOR_MAKE(180, 180, 180), /* 灰色 | Gray */
    [EXPR_CHARGING]  = LV_COLOR_MAKE(50,  220, 50),  /* 亮绿 | Bright green */
    [EXPR_ERROR]     = LV_COLOR_MAKE(255, 0,   0),   /* 纯红 | Pure red */
};

/* ================================================================
 * 背光控制 | Backlight Control
 * ================================================================ */

/**
 * @brief 初始化 PWM 背光控制
 * Initialize PWM Backlight Control
 *
 * 使用 LEDC 硬件 PWM 控制背光亮度。
 * Uses LEDC hardware PWM for backlight brightness control.
 */
static void init_backlight(void)
{
    /* LEDC 定时器配置 | LEDC timer config */
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_8_BIT,    /* 8-bit = 0~255 */
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 5000,                 /* 5 kHz */
        .clk_cfg          = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    /* LEDC 通道配置 | LEDC channel config */
    ledc_channel_config_t ledc_ch = {
        .gpio_num       = CONFIG_SPARKBOT_DISPLAY_PIN_BL,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 200,  /* 初始亮度 ~78% | Initial brightness ~78% */
        .hpoint         = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_ch));

    ESP_LOGI(TAG, "[显示] PWM 背光初始化完成 (GPIO %d) | PWM backlight initialized",
             CONFIG_SPARKBOT_DISPLAY_PIN_BL);
}

/**
 * @brief 设置背光亮度
 * Set Backlight Brightness
 */
static void set_backlight(uint8_t duty)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
}

/* ================================================================
 * LVGL 刷新回调 | LVGL Flush Callback
 * ================================================================ */

/**
 * @brief LVGL 显示刷新回调
 * LVGL Display Flush Callback
 *
 * LVGL 每帧渲染完成后调用此函数，将像素数据发送到 LCD。
 * Called after each LVGL frame render, sends pixel data to LCD.
 *
 * @param drv       显示驱动 | Display driver
 * @param area      需要刷新的矩形区域 | Rect area to refresh
 * @param color_map 像素颜色数组 | Pixel color array
 */
static void disp_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area,
                           lv_color_t *color_map)
{
    (void)drv;

    int width  = area->x2 - area->x1 + 1;
    int height = area->y2 - area->y1 + 1;

    /* 将像素数据发送到 LCD | Send pixel data to LCD */
    esp_lcd_panel_draw_bitmap(
        s_panel_handle,
        area->x1, area->y1,
        area->x1 + width, area->y1 + height,
        (const void *)color_map
    );

    /* 通知 LVGL 刷新完成 | Notify LVGL flush is done */
    lv_disp_flush_ready(drv);
}

/* ================================================================
 * GC9A01 初始化序列 | GC9A01 Init Sequence
 * ================================================================ */

/**
 * @brief 发送 GC9A01 LCD 初始化命令序列
 * Send GC9A01 LCD Initialization Command Sequence
 *
 * 基于 GC9A01 数据手册的推荐初始配置。
 * Based on recommended init config from GC9A01 datasheet.
 */
static void gc9a01_send_init_commands(void)
{
    /* GC9A01 初始化命令表 | GC9A01 init command table */
    /* 注: 简化版 — 完整初始化需要约50条命令 */
    /* Note: simplified — full init needs ~50 commands */

    /* 软件复位 | Software reset */
    esp_lcd_panel_io_tx_param(s_io_handle, 0x01, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(120));

    /* 退出睡眠 | Sleep out */
    esp_lcd_panel_io_tx_param(s_io_handle, 0x11, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(120));

    /* 设置像素格式: RGB565 */
    /* Set pixel format: RGB565 */
    uint8_t pixfmt = 0x05;  /* 16-bit/pixel */
    esp_lcd_panel_io_tx_param(s_io_handle, 0x3A, &pixfmt, 1);

    /* 显示开启 | Display on */
    esp_lcd_panel_io_tx_param(s_io_handle, 0x29, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(20));

    ESP_LOGI(TAG, "[GC9A01] 初始化命令序列发送完成 | Init commands sent");
}

/* ================================================================
 * 公共 API 实现 | Public API Implementation
 * ================================================================ */

esp_err_t lvgl_display_init(void)
{
    ESP_LOGI(TAG, "[显示] 正在初始化 LVGL + GC9A01... | Initializing LVGL + GC9A01...");

    /* ---- 1. 初始化背光 PWM | Init backlight PWM ---- */
    init_backlight();

    /* ---- 2. 分配 LVGL 显示缓冲 (PSRAM) | Allocate LVGL display buffer (PSRAM) ---- */
    s_buf1 = heap_caps_malloc(DISPLAY_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    s_buf2 = heap_caps_malloc(DISPLAY_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);

    if (s_buf1 == NULL || s_buf2 == NULL) {
        ESP_LOGE(TAG, "[显示] 分配 LVGL 缓冲区失败! | Failed to allocate LVGL buffers!");
        return ESP_ERR_NO_MEM;
    }

    /* 初始化 LVGL 显示缓冲 | Init LVGL display buffer */
    lv_disp_draw_buf_init(&s_draw_buf, s_buf1, s_buf2, DISPLAY_BUF_SIZE);

    /* ---- 3. LVGL 初始化 | LVGL init ---- */
    lv_init();

    /* ---- 4. 注册 LVGL 显示驱动 | Register LVGL display driver ---- */
    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res = DISPLAY_WIDTH;
    s_disp_drv.ver_res = DISPLAY_HEIGHT;
    s_disp_drv.flush_cb = disp_flush_cb;
    s_disp_drv.draw_buf = &s_draw_buf;
    s_disp = lv_disp_drv_register(&s_disp_drv);

    /* ---- 5. 创建默认屏幕 | Create default screen ---- */
    s_scr_main = lv_scr_act();

    /* 设置屏幕背景色为深色 (圆形屏更适合深色主题) */
    /* Set screen background to dark (round display works better with dark theme) */
    lv_obj_set_style_bg_color(s_scr_main, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s_scr_main, LV_OPA_COVER, 0);

    /* ---- 6. 创建表情文字标签 (居中) | Create expression text label (centered) ---- */
    s_label_face = lv_label_create(s_scr_main);
    lv_label_set_text(s_label_face, s_expr_strings[EXPR_NORMAL]);
    lv_obj_set_style_text_color(s_label_face, s_expr_colors[EXPR_NORMAL], 0);
    lv_obj_set_style_text_font(s_label_face, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_align(s_label_face, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(s_label_face);

    /* ---- 7. 创建气泡文字标签 (默认隐藏) | Create bubble text label (hidden by default) ---- */
    s_label_bubble = lv_label_create(s_scr_main);
    lv_label_set_text(s_label_bubble, "");
    lv_obj_set_style_text_color(s_label_bubble, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_color(s_label_bubble, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(s_label_bubble, LV_OPA_90, 0);
    lv_obj_set_style_radius(s_label_bubble, 12, 0);
    lv_obj_set_style_pad_all(s_label_bubble, 8, 0);
    lv_obj_set_style_text_font(s_label_bubble, &lv_font_montserrat_18, 0);
    lv_obj_align(s_label_bubble, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_add_flag(s_label_bubble, LV_OBJ_FLAG_HIDDEN);

    /* ---- 8. 创建状态栏 | Create status bar ---- */
    /* 电池图标 */
    s_bar_battery = lv_bar_create(s_scr_main);
    lv_obj_set_size(s_bar_battery, 40, 8);
    lv_obj_align(s_bar_battery, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_bar_set_range(s_bar_battery, 0, 100);
    lv_bar_set_value(s_bar_battery, 80, LV_ANIM_OFF);

    /* ---- 9. 创建警告标签 (默认隐藏) | Create warning label (hidden by default) ---- */
    s_label_warning = lv_label_create(s_scr_main);
    lv_obj_set_style_text_color(s_label_warning, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_text_font(s_label_warning, &lv_font_montserrat_20, 0);
    lv_obj_center(s_label_warning);
    lv_obj_add_flag(s_label_warning, LV_OBJ_FLAG_HIDDEN);

    ESP_LOGI(TAG, "[显示] LVGL 显示系统初始化完成 | LVGL display system ready");
    return ESP_OK;
}

void lvgl_display_show_boot_logo(void)
{
    /* 显示启动 Logo — 名称 + 版本 */
    /* Show boot logo — name + version */
    lv_label_set_text(s_label_face, "Spark\nBot");
    lv_obj_set_style_text_color(s_label_face, lv_color_hex(0x00FF88), 0);
    lv_obj_center(s_label_face);

    ESP_LOGI(TAG, "[显示] 启动 Logo 显示 | Boot logo displayed");
}

void lvgl_display_set_expression(sparkbot_expression_t expr)
{
    if (expr >= EXPR_COUNT) {
        ESP_LOGW(TAG, "[显示] 无效的表情ID: %d | Invalid expression ID: %d", expr);
        return;
    }

    s_current_expr = expr;

    /* 更新表情文字 | Update expression text */
    lv_label_set_text(s_label_face, s_expr_strings[expr]);

    /* 更新表情颜色 | Update expression color */
    lv_obj_set_style_text_color(s_label_face, s_expr_colors[expr], 0);

    /* 确保表情标签可见 (可能被 Logo 或警告覆盖) */
    /* Ensure expression label is visible (might be covered by logo or warning) */
    lv_obj_clear_flag(s_label_face, LV_OBJ_FLAG_HIDDEN);

    /* 隐藏警告标签 | Hide warning label */
    if (!lv_obj_has_flag(s_label_warning, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(s_label_warning, LV_OBJ_FLAG_HIDDEN);
    }

    ESP_LOGD(TAG, "[显示] 表情切换: %d -> '%s' | Expression changed: %d -> '%s'",
             expr, s_expr_strings[expr]);
}

sparkbot_expression_t lvgl_display_get_expression(void)
{
    return s_current_expr;
}

void lvgl_display_show_text_bubble(const char *text, uint32_t duration_ms)
{
    if (text == NULL || strlen(text) == 0) {
        return;
    }

    /* 更新气泡文字 | Update bubble text */
    lv_label_set_text(s_label_bubble, text);

    /* 根据文字长度调整气泡大小 | Adjust bubble size based on text length */
    size_t len = strlen(text);
    lv_obj_set_width(s_label_bubble, MIN(220, len * 12 + 20));

    /* 显示气泡 | Show bubble */
    lv_obj_clear_flag(s_label_bubble, LV_OBJ_FLAG_HIDDEN);

    /* 记录显示时间 | Record display time */
    s_bubble_show_time = esp_timer_get_time();
    s_bubble_duration = (duration_ms > 0) ? duration_ms : 5000;
    s_bubble_visible = true;

    ESP_LOGD(TAG, "[显示] 气泡: '%s' (%lums)", text, (unsigned long)duration_ms);
}

void lvgl_display_hide_text_bubble(void)
{
    lv_obj_add_flag(s_label_bubble, LV_OBJ_FLAG_HIDDEN);
    s_bubble_visible = false;
}

void lvgl_display_show_warning(const char *warning_text)
{
    if (warning_text == NULL) return;

    lv_label_set_text(s_label_warning, warning_text);
    lv_obj_clear_flag(s_label_warning, LV_OBJ_FLAG_HIDDEN);

    /* 隐藏表情标签 | Hide expression label */
    lv_obj_add_flag(s_label_face, LV_OBJ_FLAG_HIDDEN);

    ESP_LOGW(TAG, "[显示] 警告: %s | Warning: %s", warning_text);
}

void lvgl_display_update_status_bar(uint32_t battery_mv, int8_t wifi_rssi)
{
    /* 更新电池电量条 | Update battery bar */
    /* 锂电池电压范围: 3.3V (空) ~ 4.2V (满) */
    /* Li-ion battery voltage range: 3.3V (empty) ~ 4.2V (full) */
    int battery_pct = (int)((battery_mv - 3300) * 100 / (4200 - 3300));
    battery_pct = CLAMP(battery_pct, 0, 100);

    lv_bar_set_value(s_bar_battery, battery_pct, LV_ANIM_OFF);

    /* 根据电量百分比改变颜色 | Change color based on battery % */
    lv_color_t bat_color;
    if (battery_pct > 50) {
        bat_color = lv_color_hex(0x00FF00);  /* 绿色 | Green */
    } else if (battery_pct > 20) {
        bat_color = lv_color_hex(0xFFAA00);  /* 橙色 | Orange */
    } else {
        bat_color = lv_color_hex(0xFF0000);  /* 红色 | Red */
    }
    lv_obj_set_style_bg_color(s_bar_battery, bat_color, LV_PART_INDICATOR);
}

void lvgl_display_update_animation(void)
{
    /* 检查气泡超时 | Check bubble timeout */
    if (s_bubble_visible) {
        int64_t elapsed = esp_timer_get_time() - s_bubble_show_time;
        if (elapsed >= (int64_t)s_bubble_duration * 1000) {
            lvgl_display_hide_text_bubble();
        }
    }

    /* 动画帧更新 (预留) | Animation frame update (reserved)
     *
     * 如需序列帧动画效果:
     * For sprite animation effects:
     *   - 在此切换帧图片 | Switch frame images here
     *   - 调用 lv_img_set_src() 更新图片源 | Call lv_img_set_src() to update image source
     *   - 通过 ESP_LOG 记录调试信息 | Log debug info via ESP_LOG
     */
}

void lvgl_display_set_brightness(uint8_t brightness)
{
    if (brightness > 100) brightness = 100;

    s_brightness = brightness;

    if (!s_sleeping) {
        /* 将 0-100 转换为 0-255 的 PWM 占空比 */
        /* Convert 0-100 to 0-255 PWM duty */
        uint8_t duty = (uint8_t)((uint32_t)brightness * 255 / 100);
        set_backlight(duty);
    }
}

void lvgl_display_sleep(bool sleep)
{
    s_sleeping = sleep;

    if (sleep) {
        /* 关闭背光 | Turn off backlight */
        set_backlight(0);
        ESP_LOGI(TAG, "[显示] 进入休眠 (背光关闭) | Entering sleep (backlight off)");
    } else {
        /* 恢复背光 | Restore backlight */
        uint8_t duty = (uint8_t)((uint32_t)s_brightness * 255 / 100);
        set_backlight(duty);
        ESP_LOGI(TAG, "[显示] 唤醒 (背光恢复 %d%%) | Waking up (backlight restored %d%%)",
                 s_brightness);
    }
}

void lvgl_display_show_game_ui(uint8_t game_type)
{
    ESP_LOGI(TAG, "[显示] 游戏 UI: 类型 %d | Game UI: type %d", game_type);

    /* 隐藏表情, 显示游戏标题 | Hide expression, show game title */
    lv_obj_add_flag(s_label_face, LV_OBJ_FLAG_HIDDEN);

    switch (game_type) {
        case 0:  /* 2048 */
            lv_label_set_text(s_label_bubble, "2048 游戏");
            break;
        case 1:  /* 骰子 | Dice */
            lv_label_set_text(s_label_bubble, "骰子游戏");
            break;
        case 2:  /* 木鱼 | Wooden fish */
            lv_label_set_text(s_label_bubble, "电子木鱼");
            break;
        default:
            lv_label_set_text(s_label_bubble, "游戏");
            break;
    }

    lv_obj_clear_flag(s_label_bubble, LV_OBJ_FLAG_HIDDEN);
    lv_obj_center(s_label_bubble);
}

void lvgl_display_hide_game_ui(void)
{
    lv_obj_clear_flag(s_label_face, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_label_bubble, LV_OBJ_FLAG_HIDDEN);

    ESP_LOGI(TAG, "[显示] 游戏 UI 已隐藏 | Game UI hidden");
}
