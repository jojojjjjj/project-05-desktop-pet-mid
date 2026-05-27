/**
 * ================================================================
 * ESP-SparkBot -- 工具函数
 * ESP-SparkBot -- Utility Functions
 * ================================================================
 *
 * 替代原来的 src/utils.py
 * Replaces the original src/utils.py
 *
 * 功能 (Functions):
 *   - JSON 解析辅助 | JSON parsing helpers
 *   - 字符串操作 | String manipulation
 *   - 时间格式化 | Time formatting
 *   - 日志宏 | Logging macros
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * 数学辅助宏 | Math Helper Macros
 * ================================================================ */

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define CLAMP(v, lo, hi)  MAX((lo), MIN((v), (hi)))
#define ARRAY_SIZE(arr)   (sizeof(arr) / sizeof((arr)[0]))

/**
 * @brief 线性插值 | Linear interpolation
 * @param start 起始值 | Start value
 * @param end   结束值 | End value
 * @param t     插值系数 (0.0 ~ 1.0) | Interpolation factor
 * @return 插值结果 | Interpolated value
 */
static inline float lerp(float start, float end, float t)
{
    return start + (end - start) * t;
}

/**
 * @brief 计算两点之间的欧几里得距离 | Euclidean distance between two points
 */
static inline float distance_2d(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

/* ================================================================
 * JSON 解析辅助 | JSON Parsing Helpers
 * ================================================================ */

/**
 * @brief 从 JSON 字符串中提取字符串值
 * Extract a string value from a JSON string
 *
 * 简化版本的 JSON 字符串提取 — 不依赖完整的 JSON 解析库。
 * Simplified JSON string extraction — doesn't depend on full JSON lib.
 *
 * @param json    JSON 字符串 | JSON string
 * @param key     要查找的键 | Key to search for
 * @param out     输出缓冲区 | Output buffer
 * @param out_len 输出缓冲区大小 | Output buffer size
 * @return ESP_OK 成功 / ESP_FAIL 失败
 */
esp_err_t json_extract_string(const char *json, const char *key,
                               char *out, size_t out_len);

/**
 * @brief 从 JSON 字符串中提取整数值
 * Extract an integer value from a JSON string
 */
esp_err_t json_extract_int(const char *json, const char *key, int *out);

/**
 * @brief 从 JSON 字符串中提取浮点数值
 * Extract a float value from a JSON string
 */
esp_err_t json_extract_float(const char *json, const char *key, float *out);

/* ================================================================
 * 字符串操作 | String Manipulation
 * ================================================================ */

/**
 * @brief 安全字符串拼接 (防止缓冲区溢出)
 * Safe string concatenation (prevents buffer overflow)
 *
 * @param dst      目标缓冲区 | Destination buffer
 * @param src      源字符串 | Source string
 * @param dst_size 目标缓冲区容量 | Destination buffer capacity
 * @return 拼接后字符串的总长度 (不包括 null 终止符) | Total length
 */
size_t str_concat_safe(char *dst, const char *src, size_t dst_size);

/**
 * @brief URL 编码
 * URL Encode
 *
 * @param src  源字符串 | Source string
 * @param dst  目标缓冲区 | Destination buffer
 * @param dst_size 目标缓冲区容量 | Destination buffer capacity
 * @return 编码后字符串长度 | Encoded string length
 */
size_t url_encode(const char *src, char *dst, size_t dst_size);

/**
 * @brief 字符串修剪 (去除首尾空格)
 * String trim (remove leading & trailing whitespace)
 */
void str_trim(char *str);

/* ================================================================
 * 时间格式化 | Time Formatting
 * ================================================================ */

/**
 * @brief 将 Unix 时间戳格式化为 "HH:MM:SS" 字符串
 * Format Unix timestamp to "HH:MM:SS" string
 *
 * @param timestamp Unix 时间戳 (秒)
 * @param buf       输出缓冲区 (至少 9 字节)
 * @param buf_size  缓冲区大小
 */
void format_time_hms(time_t timestamp, char *buf, size_t buf_size);

/**
 * @brief 将 Unix 时间戳格式化为 "YYYY-MM-DD HH:MM:SS" 字符串
 * Format Unix timestamp to "YYYY-MM-DD HH:MM:SS"
 */
void format_time_full(time_t timestamp, char *buf, size_t buf_size);

/**
 * @brief 秒数转为友好时间描述
 * Convert seconds to human-friendly time description
 *
 * 如: 65秒 → "1分钟", 3700秒 → "1小时"
 * E.g.: 65s → "1min", 3700s → "1hr"
 */
void format_time_delta(uint32_t seconds, char *buf, size_t buf_size);

/**
 * @brief 获取当前时间的可读表示
 * Get current time as readable string
 */
void get_current_time_str(char *buf, size_t buf_size);

/* ================================================================
 * 随机数 | Random Number
 * ================================================================ */

/**
 * @brief 生成 [min, max] 范围内的随机整数
 * Generate random integer in range [min, max]
 *
 * 使用 ESP32 硬件随机数生成器 (通过 esp_random)。
 * Uses ESP32 hardware RNG (via esp_random).
 */
static inline int32_t random_range(int32_t min, int32_t max)
{
    if (min >= max) return min;
    return min + (esp_random() % (max - min + 1));
}

/* ================================================================
 * 调试辅助 | Debug Helpers
 * ================================================================ */

/**
 * @brief 打印堆内存统计信息
 * Print heap memory statistics
 */
void print_heap_stats(void);

/**
 * @brief 打印任务的栈水位线 (剩余栈空间)
 * Print task stack high water mark (remaining stack space)
 *
 * @param task_handle 任务句柄 (NULL = 当前任务)
 *                    Task handle (NULL = current task)
 */
void print_task_stack_info(TaskHandle_t task_handle);

/**
 * @brief 安全的内存分配 (带错误检查)
 * Safe memory allocation (with error checking)
 *
 * 在分配失败时打印错误日志并返回 NULL。
 * Prints error log and returns NULL on allocation failure.
 */
void *safe_malloc(size_t size);

/**
 * @brief 安全的内存释放
 * Safe memory free (sets pointer to NULL after free)
 */
#define SAFE_FREE(ptr)  do { free(ptr); (ptr) = NULL; } while(0)

/* ================================================================
 * GPIO 辅助 | GPIO Helpers
 * ================================================================ */

/**
 * @brief 初始化 GPIO 输出引脚
 * Initialize a GPIO output pin
 */
esp_err_t gpio_output_init(uint8_t pin);

/**
 * @brief 初始化 GPIO 输入引脚 (带内部上拉)
 * Initialize a GPIO input pin (with internal pull-up)
 */
esp_err_t gpio_input_pullup_init(uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H */
