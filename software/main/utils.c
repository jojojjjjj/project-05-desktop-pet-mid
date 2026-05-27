/**
 * ================================================================
 * ESP-SparkBot -- 工具函数实现
 * ESP-SparkBot -- Utility Functions Implementation
 * ================================================================
 *
 * 替代原来的 src/utils.py
 * Replaces the original src/utils.py
 */

#include "utils.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_heap_caps.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

static const char *TAG = "utils";

/* ================================================================
 * JSON 解析 | JSON Parsing
 * ================================================================ */

esp_err_t json_extract_string(const char *json, const char *key,
                               char *out, size_t out_len)
{
    if (json == NULL || key == NULL || out == NULL || out_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    /* 构建搜索模式: "key" */
    /* Build search pattern: "key" */
    char search_key[128];
    snprintf(search_key, sizeof(search_key), "\"%s\"", key);

    /* 在 JSON 中查找键 | Find key in JSON */
    const char *pos = strstr(json, search_key);
    if (pos == NULL) {
        ESP_LOGW(TAG, "[JSON] 未找到键 '%s' | Key '%s' not found", key, key);
        return ESP_FAIL;
    }

    /* 跳过键和冒号, 找到值的起始引号 */
    /* Skip key and colon, find value's opening quote */
    pos = strchr(pos + strlen(search_key), ':');
    if (pos == NULL) return ESP_FAIL;

    /* 跳过冒号和空白 | Skip colon and whitespace */
    pos++;
    while (*pos == ' ' || *pos == '\t' || *pos == '\n' || *pos == '\r') pos++;

    /* 找到值的起始引号 | Find value's opening quote */
    const char *start = strchr(pos, '"');
    if (start == NULL) return ESP_FAIL;
    start++;  /* 跳过引号 | Skip quote */

    /* 找到值的结束引号 | Find value's closing quote */
    const char *end = strchr(start, '"');
    if (end == NULL) return ESP_FAIL;

    /* 复制值到输出缓冲区 | Copy value to output buffer */
    size_t len = (size_t)(end - start);
    if (len >= out_len) {
        len = out_len - 1;
    }
    memcpy(out, start, len);
    out[len] = '\0';

    return ESP_OK;
}

esp_err_t json_extract_int(const char *json, const char *key, int *out)
{
    char str_val[32];
    esp_err_t ret = json_extract_string(json, key, str_val, sizeof(str_val));
    if (ret != ESP_OK) return ret;

    *out = atoi(str_val);
    return ESP_OK;
}

esp_err_t json_extract_float(const char *json, const char *key, float *out)
{
    char str_val[32];
    esp_err_t ret = json_extract_string(json, key, str_val, sizeof(str_val));
    if (ret != ESP_OK) return ret;

    *out = atof(str_val);
    return ESP_OK;
}

/* ================================================================
 * 字符串操作 | String Manipulation
 * ================================================================ */

size_t str_concat_safe(char *dst, const char *src, size_t dst_size)
{
    size_t dst_len = strlen(dst);
    size_t src_len = strlen(src);

    if (dst_len + src_len >= dst_size) {
        /* 目标缓冲区不够, 只复制能放得下的部分 */
        /* Destination buffer too small, copy what fits */
        size_t copy_len = dst_size - dst_len - 1;
        if (copy_len > 0) {
            memcpy(dst + dst_len, src, copy_len);
            dst[dst_size - 1] = '\0';
        }
        return dst_len + copy_len;
    }

    strcpy(dst + dst_len, src);
    return dst_len + src_len;
}

size_t url_encode(const char *src, char *dst, size_t dst_size)
{
    static const char hex[] = "0123456789ABCDEF";
    size_t i = 0, j = 0;

    while (src[i] != '\0' && j < dst_size - 1) {
        char c = src[i];
        if (('A' <= c && c <= 'Z') ||
            ('a' <= c && c <= 'z') ||
            ('0' <= c && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            /* 无需编码的字符 | Characters that don't need encoding */
            dst[j++] = c;
        } else if (c == ' ') {
            dst[j++] = '+';
        } else {
            if (j + 3 >= dst_size) break;
            dst[j++] = '%';
            dst[j++] = hex[(uint8_t)c >> 4];
            dst[j++] = hex[(uint8_t)c & 0x0F];
        }
        i++;
    }
    dst[j] = '\0';
    return j;
}

void str_trim(char *str)
{
    if (str == NULL || *str == '\0') return;

    /* 去除尾部空格 | Trim trailing spaces */
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    /* 去除首部空格 | Trim leading spaces */
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

/* ================================================================
 * 时间格式化 | Time Formatting
 * ================================================================ */

void format_time_hms(time_t timestamp, char *buf, size_t buf_size)
{
    struct tm timeinfo;
    localtime_r(&timestamp, &timeinfo);
    strftime(buf, buf_size, "%H:%M:%S", &timeinfo);
}

void format_time_full(time_t timestamp, char *buf, size_t buf_size)
{
    struct tm timeinfo;
    localtime_r(&timestamp, &timeinfo);
    strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S", &timeinfo);
}

void format_time_delta(uint32_t seconds, char *buf, size_t buf_size)
{
    if (seconds < 60) {
        snprintf(buf, buf_size, "%lu秒 | %lus", (unsigned long)seconds, (unsigned long)seconds);
    } else if (seconds < 3600) {
        snprintf(buf, buf_size, "%lu分钟 | %lumin",
                 (unsigned long)(seconds / 60), (unsigned long)(seconds / 60));
    } else if (seconds < 86400) {
        snprintf(buf, buf_size, "%lu小时 | %luhr",
                 (unsigned long)(seconds / 3600), (unsigned long)(seconds / 3600));
    } else {
        snprintf(buf, buf_size, "%lu天 | %ludays",
                 (unsigned long)(seconds / 86400), (unsigned long)(seconds / 86400));
    }
}

void get_current_time_str(char *buf, size_t buf_size)
{
    time_t now = time(NULL);
    format_time_full(now, buf, buf_size);
}

/* ================================================================
 * 调试辅助 | Debug Helpers
 * ================================================================ */

void print_heap_stats(void)
{
    size_t free_heap     = esp_get_free_heap_size();
    size_t min_free_heap = esp_get_minimum_free_heap_size();
    size_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

    ESP_LOGI(TAG, "[堆内存] 空闲: %u / 最小空闲: %u / 最大连续块: %u | "
             "Heap free: %u / min_free: %u / max_block: %u",
             free_heap, min_free_heap, largest_block,
             free_heap, min_free_heap, largest_block);
}

void print_task_stack_info(TaskHandle_t task_handle)
{
    if (task_handle == NULL) {
        task_handle = xTaskGetCurrentTaskHandle();
    }

    UBaseType_t watermark = uxTaskGetStackHighWaterMark(task_handle);
    const char *task_name = pcTaskGetName(task_handle);

    ESP_LOGI(TAG, "[栈] 任务 '%s' 栈水位: %u 字 (%u 字节) | "
             "Task '%s' stack HWM: %u words (%u bytes)",
             task_name, watermark, watermark * 4,
             task_name, watermark, watermark * 4);
}

void *safe_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL) {
        ESP_LOGE(TAG, "[内存] 分配失败! 请求大小: %u | Allocation failed! Requested: %u",
                 size);
    } else {
        memset(ptr, 0, size);  /* 零初始化 | Zero initialize */
    }
    return ptr;
}

/* ================================================================
 * GPIO 辅助 | GPIO Helpers
 * ================================================================ */

esp_err_t gpio_output_init(uint8_t pin)
{
    gpio_config_t cfg = {
        .pin_bit_mask = BIT64(pin),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    return gpio_config(&cfg);
}

esp_err_t gpio_input_pullup_init(uint8_t pin)
{
    gpio_config_t cfg = {
        .pin_bit_mask = BIT64(pin),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    return gpio_config(&cfg);
}
