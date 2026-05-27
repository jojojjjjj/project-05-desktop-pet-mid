/**
 * ================================================================
 * ESP-SparkBot -- AI 对话管理器
 * ESP-SparkBot -- AI Dialogue Manager
 * ================================================================
 *
 * 替代原来的 src/dialog.py
 * Replaces the original src/dialog.py
 *
 * 功能 (Functions):
 *   - HTTP 客户端调用多个 LLM API
 *   - 支持: DeepSeek, 百度文心一言, 火山引擎, OpenAI
 *   - 对话上下文管理 (多轮对话)
 *   - 流式响应处理
 *   - TTS 集成触发
 *
 * HTTP client for multiple LLM APIs.
 * Supports: DeepSeek, Baidu Wenxin, ByteDance Huoshan, OpenAI.
 * Conversation context management (multi-turn dialogue).
 * Streaming response handling. TTS integration trigger.
 */

#ifndef LLM_DIALOG_H
#define LLM_DIALOG_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * AI 提供商枚举 | AI Provider Enumeration
 * ================================================================ */

/**
 * @brief 支持的 AI 服务提供商
 * Supported AI Service Providers
 */
typedef enum {
    LLM_PROVIDER_DEEPSEEK = 0,  /* DeepSeek Chat */
    LLM_PROVIDER_WENXIN   = 1,  /* 百度文心一言 | Baidu Wenxin Yiyan */
    LLM_PROVIDER_HUOSHAN  = 2,  /* 火山引擎 (豆包) | ByteDance Huoshan (Doubao) */
    LLM_PROVIDER_OPENAI   = 3,  /* OpenAI ChatGPT */
    LLM_PROVIDER_CUSTOM   = 4,  /* 自定义端点 | Custom endpoint */
} llm_provider_t;

/* ================================================================
 * 对话配置结构体 | Dialogue Configuration Structure
 * ================================================================ */

/**
 * @brief AI 对话配置
 * AI Dialogue Configuration
 */
typedef struct {
    llm_provider_t provider;     /* AI 提供商 | AI provider */
    char api_key[128];           /* API 密钥 | API Key */
    char base_url[256];          /* API 端点 URL | API Endpoint URL */
    char model[64];              /* 模型名称 | Model name */
    char system_prompt[1024];    /* 系统提示词 (人设) | System prompt (persona) */

    uint16_t max_tokens;         /* 最大回复长度 | Max response tokens */
    uint8_t  temperature;        /* 创造性 (0-100, 除以100即为实际值) | Temperature (0-100, /100 = actual) */
    uint8_t  max_history_rounds; /* 最多保留对话轮数 | Max conversation rounds kept */
    uint32_t http_timeout_ms;    /* HTTP 请求超时 (毫秒) | HTTP timeout in ms */
} llm_config_t;

/* ================================================================
 * 对话消息结构 | Dialogue Message Structure
 * ================================================================ */

/**
 * @brief 单条对话消息
 * Single Dialogue Message
 */
typedef struct {
    char role[32];       /* "system" / "user" / "assistant" */
    char *content;       /* 消息内容 (动态分配) | Message content (dynamically allocated) */
    size_t content_len;  /* 消息内容长度 | Message content length */
} llm_message_t;

/* ================================================================
 * 流式响应回调 | Streaming Response Callback
 * ================================================================ */

/**
 * @brief 流式响应回调函数类型
 * Streaming Response Callback Function Type
 *
 * 每收到一个流式 token 就调用一次此回调。
 * Called once for each streaming token received.
 *
 * @param token      新收到的 token 文本 (不是 null 终止!) | Newly received token text
 * @param token_len  token 长度 | Token length
 * @param user_data  用户自定义数据 | User-defined data
 */
typedef void (*llm_stream_callback_t)(const char *token, size_t token_len,
                                       void *user_data);

/* ================================================================
 * 公共 API | Public API
 * ================================================================ */

/**
 * @brief 初始化 AI 对话管理器
 * Initialize AI Dialogue Manager
 *
 * 从 sdkconfig 加载默认配置。
 * Loads default configuration from sdkconfig.
 */
void llm_dialog_init(void);

/**
 * @brief 设置 AI 对话配置
 * Set AI Dialogue Configuration
 *
 * @param config 配置结构体指针 | Pointer to config struct
 * @return ESP_OK 成功 | ESP_OK on success
 */
esp_err_t llm_dialog_set_config(const llm_config_t *config);

/**
 * @brief 获取当前配置
 * Get Current Configuration
 *
 * @return 配置指针 (只读) | Config pointer (read-only)
 */
const llm_config_t *llm_dialog_get_config(void);

/**
 * @brief 发送对话请求 (阻塞式, 非流式)
 * Send Dialogue Request (blocking, non-streaming)
 *
 * 阻塞等待 AI 完整回复后返回。
 * Blocks until the full AI response is received.
 *
 * @param user_message 用户输入的消息 | User input message
 * @return 动态分配的 AI 回复字符串 (调用者需 free() 释放)
 *         Dynamically allocated AI response string (caller must free())
 */
char *llm_dialog_chat(const char *user_message);

/**
 * @brief 发送对话请求 (流式, 非阻塞)
 * Send Dialogue Request (streaming, non-blocking)
 *
 * 每收到一个 token 就调用一次回调函数。
 * Calls the callback function once for each received token.
 *
 * @param user_message 用户输入的消息 | User input message
 * @param callback     流式回调函数 | Streaming callback function
 * @param user_data    传递给回调的用户数据 | User data passed to callback
 * @return ESP_OK 成功 | ESP_OK on success
 */
esp_err_t llm_dialog_chat_stream(const char *user_message,
                                  llm_stream_callback_t callback,
                                  void *user_data);

/**
 * @brief 清除对话历史
 * Clear Dialogue History
 */
void llm_dialog_clear_history(void);

/**
 * @brief 设置系统提示词 (人设)
 * Set System Prompt (Persona)
 *
 * @param prompt 新的系统提示词 | New system prompt
 */
void llm_dialog_set_system_prompt(const char *prompt);

/**
 * @brief 获取对话历史长度
 * Get Dialogue History Length
 *
 * @return 当前保存的消息条数 | Number of currently saved messages
 */
uint8_t llm_dialog_get_history_count(void);

/**
 * @brief 获取宠物名字 (用于人设上下文)
 * Get Pet Name (for persona context)
 *
 * @return 宠物名字字符串 | Pet name string
 */
const char *llm_dialog_get_pet_name(void);

/**
 * @brief 设置宠物名字
 * Set Pet Name
 *
 * @param name 宠物名字 | Pet name
 */
void llm_dialog_set_pet_name(const char *name);

/**
 * @brief 构建请求 JSON (根据提供商)
 * Build Request JSON (provider-specific)
 *
 * 不同 AI 提供商的 API 请求格式不同，此函数根据当前提供商
 * 构建正确格式的 JSON 请求体。
 *
 * Different AI providers have different API request formats.
 * This function builds the correct JSON body for the current provider.
 *
 * @param messages 消息数组 | Message array
 * @param count    消息数量 | Message count
 * @param json_out 输出 JSON 字符串缓冲区 | Output JSON string buffer
 * @param buf_size 缓冲区大小 | Buffer size
 * @return ESP_OK 成功 | ESP_OK on success
 */
esp_err_t llm_dialog_build_request(const llm_message_t *messages,
                                    uint8_t count,
                                    char *json_out, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif /* LLM_DIALOG_H */
