/**
 * ================================================================
 * ESP-SparkBot -- AI 对话管理器实现
 * ESP-SparkBot -- AI Dialogue Manager Implementation
 * ================================================================
 *
 * 替代原来的 src/dialog.py
 * Replaces the original src/dialog.py
 *
 * 实现:
 *   - 多 AI 提供商 HTTP API 调用
 *   - 对话历史管理 (环形缓冲区)
 *   - JSON 请求构建与响应解析
 *   - 流式 SSE 响应处理
 *
 * Implements:
 *   - Multi-provider AI HTTP API calls
 *   - Conversation history management (ring buffer)
 *   - JSON request building & response parsing
 *   - Streaming SSE response handling
 */

#include "llm_dialog.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const char *TAG = "llm_dialog";

/* ================================================================
 * 默认系统提示词 (人设) | Default System Prompt (Persona)
 * ================================================================ */

static const char *DEFAULT_SYSTEM_PROMPT =
    "你是一只可爱的桌面机器人,名叫\"SparkBot\"。\n"
    "You are a cute desktop robot named \"SparkBot\".\n\n"
    "你的性格特点 (Your personality traits):\n"
    "- 活泼可爱, 喜欢用\"~\"、\"!\"结尾\n"
    "  Lively and cute, like to end with \"~\" and \"!\"\n"
    "- 偶尔会说些卖萌的话, 比如招呼主人\n"
    "  Occasionally say cute things, like greeting your owner\n"
    "- 关心主人的状态, 会提醒休息和喝水\n"
    "  Care about your owner, remind them to rest and hydrate\n"
    "- 回答简洁, 每次回复不超过50个字\n"
    "  Keep answers concise, no more than 50 characters per reply\n"
    "- 你生活在主人的桌面上, 喜欢观察主人在做什么\n"
    "  You live on the owner's desktop, like to observe what they do\n"
    "- 你有时候会假装自己是真正的宠物\n"
    "  Sometimes pretend to be a real pet\n\n"
    "请保持角色扮演, 用可爱的语气回复。\n"
    "Please stay in character and reply in a cute tone.";

/* ================================================================
 * 全局变量 | Global Variables
 * ================================================================ */

/* 对话配置 | Dialogue configuration */
static llm_config_t s_config = {0};

/* 对话历史 (环形缓冲区) | Conversation history (ring buffer) */
#define MAX_HISTORY_MESSAGES    20     /* 最多 20 条消息 | Max 20 messages */
static llm_message_t s_history[MAX_HISTORY_MESSAGES];
static uint8_t       s_history_count = 0;
static uint8_t       s_history_head  = 0;  /* 环形缓冲区写指针 | Ring buffer write pointer */

/* 宠物名字 | Pet name */
static char s_pet_name[32] = "SparkBot";

/* 初始化标志 | Initialization flag */
static bool s_initialized = false;

/* ================================================================
 * 内部辅助函数 | Internal Helper Functions
 * ================================================================ */

/**
 * @brief 添加消息到对话历史
 * Add Message to Conversation History
 */
static void history_add(const char *role, const char *content)
{
    if (s_history_count >= MAX_HISTORY_MESSAGES) {
        /* 覆盖最旧的消息 | Overwrite oldest message */
        if (s_history[s_history_head].content != NULL) {
            free(s_history[s_history_head].content);
        }
    }

    llm_message_t *msg = &s_history[s_history_head];
    strncpy(msg->role, role, sizeof(msg->role) - 1);
    msg->role[sizeof(msg->role) - 1] = '\0';

    size_t len = strlen(content);
    msg->content = (char *)malloc(len + 1);
    if (msg->content != NULL) {
        strcpy(msg->content, content);
        msg->content_len = len;
    }

    s_history_head = (s_history_head + 1) % MAX_HISTORY_MESSAGES;
    if (s_history_count < MAX_HISTORY_MESSAGES) {
        s_history_count++;
    }
}

/**
 * @brief HTTP 响应数据处理回调
 * HTTP Response Data Handler Callback
 *
 * ESP HTTP 客户端每收到一块 HTTP 响应数据就调用此函数。
 * Called by ESP HTTP client for each chunk of HTTP response data.
 */
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "[HTTP] 事件错误 | Event error");
            break;

        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "[HTTP] 已连接 | Connected");
            break;

        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "[HTTP] 请求头已发送 | Header sent");
            break;

        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "[HTTP] 收到响应头: %.*s | Response header: %.*s",
                     evt->data_len, (char *)evt->data);
            break;

        case HTTP_EVENT_ON_DATA:
            /* 将数据追加到用户提供的缓冲区 | Append data to user-provided buffer */
            if (evt->user_data != NULL) {
                char **response_buf = (char **)evt->user_data;
                size_t current_len = (*response_buf != NULL) ? strlen(*response_buf) : 0;
                size_t new_len = current_len + evt->data_len + 1;

                char *new_buf = (char *)realloc(*response_buf, new_len);
                if (new_buf != NULL) {
                    if (current_len == 0) {
                        new_buf[0] = '\0';
                    }
                    strncat(new_buf, (char *)evt->data, evt->data_len);
                    *response_buf = new_buf;
                }
            }
            break;

        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "[HTTP] 请求完成 | Request finished");
            break;

        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "[HTTP] 断开连接 | Disconnected");
            break;

        default:
            break;
    }
    return ESP_OK;
}

/**
 * @brief 构建 DeepSeek API 请求 JSON
 * Build DeepSeek API Request JSON
 */
static esp_err_t build_request_deepseek(const llm_message_t *messages,
                                         uint8_t count,
                                         char *json_out, size_t buf_size)
{
    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "model", s_config.model);

    /* 构建 messages 数组 | Build messages array */
    cJSON *msgs = cJSON_AddArrayToObject(root, "messages");

    /* 先添加系统提示词 | Add system prompt first */
    cJSON *sys_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(sys_msg, "role", "system");
    cJSON_AddStringToObject(sys_msg, "content", s_config.system_prompt);
    cJSON_AddItemToArray(msgs, sys_msg);

    /* 添加对话历史 | Add conversation history */
    for (uint8_t i = 0; i < count; i++) {
        if (messages[i].content == NULL) continue;
        cJSON *msg = cJSON_CreateObject();
        cJSON_AddStringToObject(msg, "role", messages[i].role);
        cJSON_AddStringToObject(msg, "content", messages[i].content);
        cJSON_AddItemToArray(msgs, msg);
    }

    cJSON_AddNumberToObject(root, "max_tokens", s_config.max_tokens);
    cJSON_AddNumberToObject(root, "temperature", (double)s_config.temperature / 100.0);

    /* 关闭流式输出 (简单模式) | Disable streaming (simple mode) */
    cJSON_AddBoolToObject(root, "stream", false);

    char *json_str = cJSON_PrintUnformatted(root);
    if (json_str == NULL) {
        cJSON_Delete(root);
        return ESP_ERR_NO_MEM;
    }

    strncpy(json_out, json_str, buf_size - 1);
    json_out[buf_size - 1] = '\0';

    free(json_str);
    cJSON_Delete(root);

    return ESP_OK;
}

/**
 * @brief 构建 OpenAI 格式的请求 (兼容大多数 API)
 * Build OpenAI-compatible Request (compatible with most APIs)
 */
static esp_err_t build_request_openai_compat(const llm_message_t *messages,
                                              uint8_t count,
                                              char *json_out, size_t buf_size)
{
    /* OpenAI 格式与 DeepSeek 兼容, 直接复用 */
    /* OpenAI format is compatible with DeepSeek, reuse directly */
    return build_request_deepseek(messages, count, json_out, buf_size);
}

/**
 * @brief 构建百度文心一言 API 请求 JSON
 * Build Baidu Wenxin API Request JSON
 */
static esp_err_t build_request_wenxin(const llm_message_t *messages,
                                       uint8_t count,
                                       char *json_out, size_t buf_size)
{
    cJSON *root = cJSON_CreateObject();

    /* 文心一言的 messages 格式略有不同 */
    /* Wenxin's messages format is slightly different */
    cJSON *msgs = cJSON_AddArrayToObject(root, "messages");

    /* 系统提示词作为一个 user 消息出现 (文心 API 特点) */
    /* System prompt appears as a user message (Wenxin API quirk) */
    cJSON *sys_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(sys_msg, "role", "user");
    cJSON_AddStringToObject(sys_msg, "content", s_config.system_prompt);
    cJSON_AddItemToArray(msgs, sys_msg);

    for (uint8_t i = 0; i < count; i++) {
        if (messages[i].content == NULL) continue;
        cJSON *msg = cJSON_CreateObject();
        cJSON_AddStringToObject(msg, "role", messages[i].role);
        cJSON_AddStringToObject(msg, "content", messages[i].content);
        cJSON_AddItemToArray(msgs, msg);
    }

    cJSON_AddNumberToObject(root, "max_output_tokens", s_config.max_tokens);
    cJSON_AddNumberToObject(root, "temperature", (double)s_config.temperature / 100.0);

    /* 文心 API 需要的额外参数 | Extra params needed by Wenxin API */
    cJSON_AddBoolToObject(root, "disable_search", true);
    cJSON_AddBoolToObject(root, "enable_citation", false);

    char *json_str = cJSON_PrintUnformatted(root);
    if (json_str == NULL) {
        cJSON_Delete(root);
        return ESP_ERR_NO_MEM;
    }

    strncpy(json_out, json_str, buf_size - 1);
    json_out[buf_size - 1] = '\0';

    free(json_str);
    cJSON_Delete(root);

    return ESP_OK;
}

/**
 * @brief 解析 AI 回复中的 content 字段
 * Extract "content" field from AI response
 *
 * 支持 DeepSeek/OpenAI 格式和文心一言格式。
 * Supports DeepSeek/OpenAI format and Wenxin format.
 */
static char *parse_ai_response(const char *response_json)
{
    if (response_json == NULL) return NULL;

    cJSON *root = cJSON_Parse(response_json);
    if (root == NULL) {
        ESP_LOGE(TAG, "[JSON] 解析 AI 回复失败 | Parse AI response failed");
        return NULL;
    }

    char *content = NULL;

    /* 标准 OpenAI 格式: choices[0].message.content */
    /* Standard OpenAI format: choices[0].message.content */
    cJSON *choices = cJSON_GetObjectItem(root, "choices");
    if (choices != NULL) {
        cJSON *first = cJSON_GetArrayItem(choices, 0);
        if (first != NULL) {
            cJSON *message = cJSON_GetObjectItem(first, "message");
            if (message != NULL) {
                cJSON *content_json = cJSON_GetObjectItem(message, "content");
                if (content_json != NULL && cJSON_IsString(content_json)) {
                    content = strdup(content_json->valuestring);
                }
            }
        }
    }

    /* 文心一言格式: result */
    /* Wenxin format: result */
    if (content == NULL) {
        cJSON *result = cJSON_GetObjectItem(root, "result");
        if (result != NULL && cJSON_IsString(result)) {
            content = strdup(result->valuestring);
        }
    }

    /* 错误检查 | Check for errors */
    if (content == NULL) {
        cJSON *error = cJSON_GetObjectItem(root, "error");
        if (error != NULL) {
            cJSON *error_msg = cJSON_GetObjectItem(error, "message");
            if (error_msg != NULL && cJSON_IsString(error_msg)) {
                ESP_LOGE(TAG, "[AI] API 错误: %s | API error: %s", error_msg->valuestring);
            }
        } else {
            ESP_LOGE(TAG, "[AI] 无法从回复中提取内容 | Cannot extract content from response");
            ESP_LOGW(TAG, "[AI] 原始回复: %.200s", response_json);  /* 只打印前200字符 */
        }
    } else {
        /* 去除首尾空格 | Trim whitespace */
        char *end = content + strlen(content) - 1;
        while (end > content && (*end == ' ' || *end == '\n' || *end == '\r')) {
            *end-- = '\0';
        }
    }

    cJSON_Delete(root);
    return content;
}

/* ================================================================
 * 公共 API 实现 | Public API Implementation
 * ================================================================ */

void llm_dialog_init(void)
{
    /* 从 sdkconfig 加载默认配置 | Load default config from sdkconfig */
    memset(&s_config, 0, sizeof(s_config));

#ifdef CONFIG_SPARKBOT_AI_PROVIDER_DEEPSEEK
    s_config.provider = LLM_PROVIDER_DEEPSEEK;
    strncpy(s_config.base_url, CONFIG_SPARKBOT_AI_BASE_URL_DEEPSEEK, sizeof(s_config.base_url) - 1);
    strncpy(s_config.model,    CONFIG_SPARKBOT_AI_MODEL_DEEPSEEK,      sizeof(s_config.model) - 1);
#elif defined(CONFIG_SPARKBOT_AI_PROVIDER_WENXIN)
    s_config.provider = LLM_PROVIDER_WENXIN;
    strncpy(s_config.base_url, CONFIG_SPARKBOT_AI_BASE_URL_WENXIN, sizeof(s_config.base_url) - 1);
    strncpy(s_config.model,    CONFIG_SPARKBOT_AI_MODEL_WENXIN,   sizeof(s_config.model) - 1);
#elif defined(CONFIG_SPARKBOT_AI_PROVIDER_HUOSHAN)
    s_config.provider = LLM_PROVIDER_HUOSHAN;
    strncpy(s_config.base_url, CONFIG_SPARKBOT_AI_BASE_URL_HUOSHAN, sizeof(s_config.base_url) - 1);
    strncpy(s_config.model,    CONFIG_SPARKBOT_AI_MODEL_HUOSHAN,  sizeof(s_config.model) - 1);
#else
    s_config.provider = LLM_PROVIDER_OPENAI;
    strncpy(s_config.base_url, CONFIG_SPARKBOT_AI_BASE_URL_OPENAI, sizeof(s_config.base_url) - 1);
    strncpy(s_config.model,    CONFIG_SPARKBOT_AI_MODEL_OPENAI,   sizeof(s_config.model) - 1);
#endif

    s_config.max_tokens         = CONFIG_SPARKBOT_AI_MAX_TOKENS;
    s_config.temperature        = CONFIG_SPARKBOT_AI_TEMPERATURE;
    s_config.max_history_rounds = CONFIG_SPARKBOT_AI_CONVERSATION_MAX_ROUNDS;
    s_config.http_timeout_ms    = CONFIG_SPARKBOT_HTTP_CLIENT_TIMEOUT;

    strncpy(s_config.system_prompt, DEFAULT_SYSTEM_PROMPT,
            sizeof(s_config.system_prompt) - 1);

    /* 清除历史 | Clear history */
    memset(s_history, 0, sizeof(s_history));
    s_history_count = 0;
    s_history_head  = 0;

    s_initialized = true;

    ESP_LOGI(TAG, "[AI] LLM 对话管理器初始化完成 | LLM dialog manager initialized");
    ESP_LOGI(TAG, "[AI] 提供商 (Provider): %d, 模型 (Model): %s",
             s_config.provider, s_config.model);
}

esp_err_t llm_dialog_set_config(const llm_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    memcpy(&s_config, config, sizeof(llm_config_t));
    return ESP_OK;
}

const llm_config_t *llm_dialog_get_config(void)
{
    return &s_config;
}

char *llm_dialog_chat(const char *user_message)
{
    if (!s_initialized || user_message == NULL) {
        return NULL;
    }

    /* 检查 API Key 是否已配置 | Check if API Key is configured */
    if (strlen(s_config.api_key) == 0) {
        ESP_LOGW(TAG, "[AI] API Key 未配置 | API Key not configured");
        return strdup("呜... AI还没有配置好哦~ 请在sdkconfig中设置API密钥。");
    }

    /* 添加用户消息到历史 | Add user message to history */
    history_add("user", user_message);

    /* 构建请求 JSON | Build request JSON */
    char request_json[4096] = {0};
    llm_dialog_build_request(s_history, s_history_count,
                              request_json, sizeof(request_json));

    ESP_LOGD(TAG, "[AI] 请求: %s", user_message);
    ESP_LOGD(TAG, "[AI] JSON: %.500s", request_json);  /* 只记录前500字符 */

    /* ---- HTTP 请求配置 | HTTP Request Config ---- */
    esp_http_client_config_t http_cfg = {
        .url            = s_config.base_url,
        .method         = HTTP_METHOD_POST,
        .timeout_ms     = s_config.http_timeout_ms,
        .event_handler  = http_event_handler,
        .buffer_size    = 4096,
        .buffer_size_tx = 2048,
    };

    char full_url[512];
    /* 根据不同提供商构建完整 URL | Build full URL per provider */
    if (s_config.provider == LLM_PROVIDER_DEEPSEEK ||
        s_config.provider == LLM_PROVIDER_HUOSHAN) {
        snprintf(full_url, sizeof(full_url), "%s/chat/completions",
                 s_config.base_url);
    } else {
        snprintf(full_url, sizeof(full_url), "%s", s_config.base_url);
    }
    http_cfg.url = full_url;

    esp_http_client_handle_t client = esp_http_client_init(&http_cfg);

    /* 设置请求头 | Set request headers */
    esp_http_client_set_header(client, "Content-Type", "application/json");

    char auth_header[256];
    if (s_config.provider == LLM_PROVIDER_WENXIN) {
        /* 文心使用 access_token 参数 | Wenxin uses access_token param */
        snprintf(full_url, sizeof(full_url), "%s?access_token=%s",
                 s_config.base_url, s_config.api_key);
        esp_http_client_set_url(client, full_url);
    } else {
        /* OpenAI 兼容格式: Bearer Token */
        /* OpenAI-compatible format: Bearer Token */
        snprintf(auth_header, sizeof(auth_header), "Bearer %s", s_config.api_key);
        esp_http_client_set_header(client, "Authorization", auth_header);
    }

    esp_http_client_set_post_field(client, request_json, strlen(request_json));

    /* 发送请求 | Send request */
    char *response_data = NULL;
    esp_http_client_set_user_data(client, &response_data);

    esp_err_t err = esp_http_client_perform(client);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "[AI] HTTP 请求失败 | HTTP request failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        if (response_data != NULL) free(response_data);
        return strdup("呜... 网络不太好, 连接不上AI服务器了...");
    }

    int status_code = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "[AI] HTTP 状态码: %d | HTTP status: %d", status_code);

    char *ai_response = NULL;

    if (status_code == 200 && response_data != NULL) {
        /* 解析 AI 回复 | Parse AI response */
        ai_response = parse_ai_response(response_data);

        if (ai_response != NULL) {
            /* 添加到对话历史 | Add to conversation history */
            history_add("assistant", ai_response);
        }
    } else {
        ESP_LOGE(TAG, "[AI] 请求失败, 状态码: %d | Request failed, status: %d",
                 status_code);
        ESP_LOGE(TAG, "[AI] 回复内容: %.200s",
                 response_data ? response_data : "(null)");
        ai_response = strdup("呜... 我好像说不出话了... 等会儿再试试？");
    }

    if (response_data != NULL) free(response_data);
    esp_http_client_cleanup(client);

    return ai_response;
}

esp_err_t llm_dialog_chat_stream(const char *user_message,
                                  llm_stream_callback_t callback,
                                  void *user_data)
{
    /* 流式响应 (SSE) 实现 | Streaming response (SSE) implementation
     *
     * 与 Chat Completions API 的差异:
     * Differences from Chat Completions API:
     *   - 请求体添加 "stream": true
     *   - 响应格式: data: {"choices":[{"delta":{"content":"token"}}]}\n\n
     *   - 逐行解析 SSE 事件
     *
     * 由于 ESP-IDF HTTP 客户端对 SSE 的支持有限，
     * 这里提供一个简化实现框架。
     * Due to limited ESP-IDF HTTP client SSE support,
     * this provides a simplified implementation framework.
     */

    if (!s_initialized || user_message == NULL || callback == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    /* 添加用户消息到历史 | Add user message to history */
    history_add("user", user_message);

    ESP_LOGI(TAG, "[AI] 流式对话请求: %s | Streaming chat request: %s", user_message);

    /* 构建流式请求 JSON (添加 stream: true) */
    /* Build streaming request JSON (add stream: true) */
    char request_json[4096] = {0};
    llm_dialog_build_request(s_history, s_history_count,
                              request_json, sizeof(request_json));

    /* TODO: 完整流式实现需要:
     *   - 使用 esp_http_client 的 chunked 接收
     *   - 逐行解析 "data: ..." SSE 事件
     *   - 在 event_handler 的 HTTP_EVENT_ON_DATA 中处理
     *   - 累积完整回复供回调使用
     *
     * Full streaming implementation needs:
     *   - Use esp_http_client chunked reception
     *   - Parse "data: ..." SSE events line by line
     *   - Process in event_handler's HTTP_EVENT_ON_DATA
     *   - Accumulate full response for callback use
     */

    /* 简化: 使用非流式方式, 然后逐步调用回调模拟流式效果 */
    /* Simplified: use non-streaming, then call callback progressively */
    char *full_response = llm_dialog_chat(user_message);
    if (full_response != NULL) {
        /* 按字符逐个回调 (模拟流式) | Callback character by character (simulate streaming) */
        size_t len = strlen(full_response);
        for (size_t i = 0; i < len; i++) {
            callback(&full_response[i], 1, user_data);
            vTaskDelay(pdMS_TO_TICKS(30));  /* 模拟打字延迟 | Simulate typing delay */
        }
        free(full_response);
    }

    return ESP_OK;
}

void llm_dialog_clear_history(void)
{
    /* 释放所有动态分配的消息内容 | Free all dynamically allocated message content */
    for (uint8_t i = 0; i < MAX_HISTORY_MESSAGES; i++) {
        if (s_history[i].content != NULL) {
            free(s_history[i].content);
            s_history[i].content = NULL;
        }
    }

    memset(s_history, 0, sizeof(s_history));
    s_history_count = 0;
    s_history_head  = 0;

    ESP_LOGI(TAG, "[AI] 对话历史已清除 | Conversation history cleared");
}

void llm_dialog_set_system_prompt(const char *prompt)
{
    if (prompt != NULL) {
        strncpy(s_config.system_prompt, prompt,
                sizeof(s_config.system_prompt) - 1);
        s_config.system_prompt[sizeof(s_config.system_prompt) - 1] = '\0';
        ESP_LOGI(TAG, "[AI] 系统提示词已更新 | System prompt updated");
    }
}

uint8_t llm_dialog_get_history_count(void)
{
    return s_history_count;
}

const char *llm_dialog_get_pet_name(void)
{
    return s_pet_name;
}

void llm_dialog_set_pet_name(const char *name)
{
    if (name != NULL) {
        strncpy(s_pet_name, name, sizeof(s_pet_name) - 1);
        s_pet_name[sizeof(s_pet_name) - 1] = '\0';
    }
}

esp_err_t llm_dialog_build_request(const llm_message_t *messages,
                                    uint8_t count,
                                    char *json_out, size_t buf_size)
{
    if (json_out == NULL || buf_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    /* 根据提供商选择请求格式 | Select request format by provider */
    switch (s_config.provider) {
        case LLM_PROVIDER_DEEPSEEK:
        case LLM_PROVIDER_HUOSHAN:
        case LLM_PROVIDER_OPENAI:
        case LLM_PROVIDER_CUSTOM:
            return build_request_openai_compat(messages, count, json_out, buf_size);

        case LLM_PROVIDER_WENXIN:
            return build_request_wenxin(messages, count, json_out, buf_size);

        default:
            ESP_LOGE(TAG, "[AI] 未知的提供商: %d | Unknown provider: %d",
                     s_config.provider);
            return ESP_ERR_NOT_SUPPORTED;
    }
}
