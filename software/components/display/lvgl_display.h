/**
 * ================================================================
 * ESP-SparkBot -- 显示管理组件 (LVGL)
 * ESP-SparkBot -- Display Manager Component (LVGL)
 * ================================================================
 *
 * 替代原来的 src/animation.py
 * Replaces the original src/animation.py
 *
 * 功能 (Functions):
 *   - GC9A01 圆形显示屏驱动 (240x240)
 *   - LVGL 屏幕初始化与渲染
 *   - 动画帧管理 (表情/动作切换)
 *   - UI 布局 (表情、气泡文字、状态栏)
 *
 * GC9A01 round display driver (240x240).
 * LVGL screen initialization, rendering, animation frames,
 * and UI layouts.
 */

#ifndef LVGL_DISPLAY_H
#define LVGL_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * 显示参数 | Display Parameters
 * ================================================================ */

#define DISPLAY_WIDTH   240    /* 屏幕宽度 | Screen width */
#define DISPLAY_HEIGHT  240    /* 屏幕高度 | Screen height */
#define DISPLAY_BUF_SIZE (DISPLAY_WIDTH * 40)  /* 每行缓冲 | Per-line buffer */

/* ================================================================
 * 表情/动画状态索引 | Expression / Animation State Index
 * ================================================================ */

/**
 * @brief 机器人表情枚举
 * Robot Expression Enumeration
 *
 * 每种表情对应一组预设的 LVGL 对象 (文字/图片/动画)。
 * Each expression maps to a set of preset LVGL objects (text/image/animation).
 */
typedef enum {
    EXPR_NORMAL     = 0,  /* 正常表情 (^_^) */
    EXPR_HAPPY      = 1,  /* 开心 (≧▽≦) */
    EXPR_SAD        = 2,  /* 难过 (╥_╥) */
    EXPR_SURPRISED  = 3,  /* 惊讶 (○_○) */
    EXPR_ANGRY      = 4,  /* 生气 (-_-)* */
    EXPR_SLEEPY     = 5,  /* 困 (－_－)zZZ */
    EXPR_LOVE       = 6,  /* 爱心 (♥) */
    EXPR_LISTENING  = 7,  /* 聆听 (竖耳朵) | Listening (ears up) */
    EXPR_SPEAKING   = 8,  /* 说话 (嘴巴动) | Speaking (mouth moving) */
    EXPR_THINKING   = 9,  /* 思考 (...?) */
    EXPR_CHARGING   = 10, /* 充电 (⚡) */
    EXPR_ERROR      = 11, /* 故障 (╳_╳) */
    EXPR_COUNT      = 12  /* 表情总数 | Total expressions */
} sparkbot_expression_t;

/* ================================================================
 * 公共 API | Public API
 * ================================================================ */

/**
 * @brief 初始化 LVGL 显示系统
 * Initialize LVGL Display System
 *
 * 执行步骤 | Steps:
 *   1. 注册 LVGL 显示驱动 | Register LVGL display driver
 *   2. 配置 GC9A01 SPI LCD 控制器 | Configure GC9A01 SPI LCD controller
 *   3. 创建默认 UI 屏幕 | Create default UI screen
 *   4. 创建动画定时器 | Create animation timer
 *
 * @return ESP_OK 成功 | ESP_OK on success
 */
esp_err_t lvgl_display_init(void);

/**
 * @brief 显示启动 Logo
 * Display Boot Logo
 */
void lvgl_display_show_boot_logo(void);

/**
 * @brief 切换到指定表情
 * Switch to specified expression
 *
 * @param expr 表情枚举值 | Expression enum value
 */
void lvgl_display_set_expression(sparkbot_expression_t expr);

/**
 * @brief 获取当前表情
 * Get current expression
 *
 * @return 当前表情 | Current expression
 */
sparkbot_expression_t lvgl_display_get_expression(void);

/**
 * @brief 显示文字气泡
 * Display Text Bubble
 *
 * 在机器人"头顶"显示一段圆形气泡文字。
 * Shows a round bubble text above the robot "head".
 *
 * @param text         要显示的文字 | Text to display
 * @param duration_ms  显示持续时间 (毫秒), 0 = 永久
 *                     Display duration in ms, 0 = forever
 */
void lvgl_display_show_text_bubble(const char *text, uint32_t duration_ms);

/**
 * @brief 隐藏文字气泡
 * Hide Text Bubble
 */
void lvgl_display_hide_text_bubble(void);

/**
 * @brief 显示警告图标 (如低电量)
 * Display Warning Icon (e.g. low battery)
 *
 * @param warning_text 警告文字 | Warning text
 */
void lvgl_display_show_warning(const char *warning_text);

/**
 * @brief 显示状态栏信息
 * Display Status Bar Information
 *
 * @param battery_mv 电池电量 (mV) | Battery voltage in mV
 * @param wifi_rssi  WiFi 信号强度 (dBm) | WiFi RSSI in dBm
 */
void lvgl_display_update_status_bar(uint32_t battery_mv, int8_t wifi_rssi);

/**
 * @brief 动画帧更新 (由显示任务周期性调用)
 * Animation Frame Update (called periodically by display task)
 *
 * 驱动当前播放的动画到下一帧。
 * Drives the currently playing animation to the next frame.
 */
void lvgl_display_update_animation(void);

/**
 * @brief 设置屏幕亮度
 * Set Screen Brightness
 *
 * @param brightness 亮度 0-100 | Brightness 0-100
 */
void lvgl_display_set_brightness(uint8_t brightness);

/**
 * @brief 屏幕休眠 / 唤醒
 * Screen Sleep / Wake
 *
 * @param sleep true=关闭背光进入休眠, false=恢复
 *              true=turn off backlight to sleep, false=restore
 */
void lvgl_display_sleep(bool sleep);

/**
 * @brief 显示简单的游戏界面 (2048, 骰子等)
 * Display Simple Game UI (2048, dice, etc.)
 *
 * @param game_type 游戏类型 ID | Game type ID
 */
void lvgl_display_show_game_ui(uint8_t game_type);

/**
 * @brief 隐藏游戏界面
 * Hide Game UI
 */
void lvgl_display_hide_game_ui(void);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_DISPLAY_H */
