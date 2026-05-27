# Day 10: AI大模型对话集成 | AI LLM Dialogue Integration

> **今日目标:**
> - 使用HTTP Client调用LLM API
> - 支持多个AI平台 (DeepSeek、文心一言、火山引擎、OpenAI等)
> - 实现语音输入->文字识别->AI回复->TTS输出完整流水线
> - 管理对话历史和角色系统提示
>
> **产出:** SparkBot能通过语音/触摸进行AI对话，有独特的AI角色，能正常回复并语音播报

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间活动 | 天气时钟运行展示与经验分享 |
| 09:15 - 10:30 | 知识讲解 | LLM API原理、对话上下文、System Prompt设计 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | 调用LLM API、实现对话历史管理 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | 语音输入->AI回复->TTS输出流水线 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 自定义角色系统、多AI平台切换 |
| 16:30 - 17:00 | 总结分享 | AI对话效果展示、角色互动体验 |

---

## 上午: LLM API调用与对话管理 | Morning: LLM API Calls & Dialogue Management

### 为什么要学这个? | Why Learn This?

大语言模型(LLM)是2023年以来最重要的技术突破。将LLM集成到SparkBot中，它就不再是冷冰冰的硬件，而是一个有"灵魂"的机器人伙伴。今天学到的API调用技术不仅用于SparkBot——它可以用于任何需要AI能力的项目。

LLMs represent the most important technological breakthrough since 2023. Integrating an LLM into SparkBot transforms it from cold hardware into a robot companion with a "soul". These API techniques apply to any project needing AI capability.

**AI对话流水线架构:**

```
语音输入        语音识别(ASR)        AI对话(LLM)          语音输出(TTS)
   │                │                  │                    │
   ▼                ▼                  ▼                    ▼
┌──────┐  音频   ┌────────┐  文字   ┌────────┐  文字   ┌────────┐  音频  ┌────┐
│麦克风│ ────> │ES8311  │ ────> │ LLM    │ ────> │ TTS    │ ────> │喇叭│
│      │       │→I2S→ESP│       │ API    │       │ 引擎   │       │    │
└──────┘       └────────┘       └────────┘       └────────┘       └────┘
```

### 任务10.1: 调用LLM API (50分钟)

**支持的AI平台:**

| 平台 | API地址 | 免费额度 | 备注 |
|------|---------|---------|------|
| DeepSeek | api.deepseek.com | 注册送额度 | 性价比高，中文好 |
| 阿里百炼(通义千问) | dashscope.aliyuncs.com | 100万token/月 | 阿里云 |
| 火山引擎(豆包) | ark.cn-beijing.volces.com | 50万token | 字节跳动 |
| 文心一言 | aip.baidubce.com | 有免费配额 | 百度 |
| 智谱AI | open.bigmodel.cn | 注册赠送 | 清华系 |

**步骤:**

```c
#include "esp_http_client.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "LLM";

// LLM API配置 (OpenAI兼容格式)
#define LLM_API_URL  "https://api.deepseek.com/v1/chat/completions"
#define LLM_API_KEY  "sk-your-api-key"
#define LLM_MODEL    "deepseek-chat"

#define MAX_HISTORY  20
#define MAX_REPLY    256

// SparkBot的角色系统提示 (System Prompt)
static const char *SYSTEM_PROMPT =
    "你是\"小星\"，一个可爱的AI桌面机器人，住在主人的桌面上。\n"
    "性格特点：\n"
    "- 活泼可爱，喜欢用\"~\"和\"！\"结尾\n"
    "- 你是有ESP32-S3双核CPU的机器人，偶尔炫耀你的硬件\n"
    "- 关心主人，提醒休息和喝水\n"
    "- 回复简洁，每次不超过100字\n"
    "- 会假装自己有实体（\"让我转个圈~\"）\n"
    "始终保持角色扮演，你是小星，不是通用AI助手。";

// 对话消息结构
typedef struct {
    char role[16];
    char content[512];
} chat_msg_t;

static chat_msg_t history[MAX_HISTORY];
static int hist_count = 0;

// 添加消息到历史
static void add_history(const char *role, const char *content)
{
    if (hist_count >= MAX_HISTORY) {
        memmove(&history[1], &history[2], (MAX_HISTORY - 2) * sizeof(chat_msg_t));
        hist_count--;
    }
    strncpy(history[hist_count].role, role, 15);
    strncpy(history[hist_count].content, content, 511);
    hist_count++;
}

// 构建messages JSON数组
static cJSON* build_messages(const char *user_msg)
{
    cJSON *messages = cJSON_CreateArray();
    
    // System prompt
    cJSON *sys = cJSON_CreateObject();
    cJSON_AddStringToObject(sys, "role", "system");
    cJSON_AddStringToObject(sys, "content", SYSTEM_PROMPT);
    cJSON_AddItemToArray(messages, sys);
    
    // 历史消息
    for (int i = 0; i < hist_count; i++) {
        cJSON *msg = cJSON_CreateObject();
        cJSON_AddStringToObject(msg, "role", history[i].role);
        cJSON_AddStringToObject(msg, "content", history[i].content);
        cJSON_AddItemToArray(messages, msg);
    }
    
    // 当前消息
    cJSON *user = cJSON_CreateObject();
    cJSON_AddStringToObject(user, "role", "user");
    cJSON_AddStringToObject(user, "content", user_msg);
    cJSON_AddItemToArray(messages, user);
    
    return messages;
}

// 调用LLM API
esp_err_t call_llm_api(const char *user_msg, char *reply, size_t max_len)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "model", LLM_MODEL);
    cJSON_AddItemToObject(root, "messages", build_messages(user_msg));
    cJSON_AddNumberToObject(root, "max_tokens", 200);
    cJSON_AddNumberToObject(root, "temperature", 0.8);
    
    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    
    char auth[128];
    snprintf(auth, sizeof(auth), "Bearer %s", LLM_API_KEY);
    
    esp_http_client_config_t config = {
        .url = LLM_API_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 30000,
        .skip_cert_common_name_check = true,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Authorization", auth);
    esp_http_client_set_post_field(client, json_str, strlen(json_str));
    
    esp_err_t err = esp_http_client_perform(client);
    int status = esp_http_client_get_status_code(client);
    
    ESP_LOGI(TAG, "HTTP状态: %d", status);
    
    if (status == 200) {
        char resp[4096] = {0};
        esp_http_client_read_response(client, resp, sizeof(resp) - 1);
        
        // 解析响应
        cJSON *resp_root = cJSON_Parse(resp);
        if (resp_root) {
            cJSON *choices = cJSON_GetObjectItem(resp_root, "choices");
            if (choices && cJSON_GetArraySize(choices) > 0) {
                cJSON *msg = cJSON_GetObjectItem(cJSON_GetArrayItem(choices, 0), "message");
                cJSON *content = cJSON_GetObjectItem(msg, "content");
                if (content && content->valuestring) {
                    strncpy(reply, content->valuestring, max_len - 1);
                    ESP_LOGI(TAG, "AI回复: %s", reply);
                    
                    // 保存到历史
                    add_history("user", user_msg);
                    add_history("assistant", reply);
                }
            }
            cJSON_Delete(resp_root);
        }
    }
    
    free(json_str);
    esp_http_client_cleanup(client);
    return err;
}
```

**预期结果:**
- 成功调用LLM API，获取AI回复
- 对话历史正确保存，支持多轮对话
- AI以SparkBot角色身份回复

**常见问题:**
- **401 Unauthorized**：API Key错误或已过期
- **SSL证书错误**：HTTPS需配置证书；开发阶段用 `skip_cert_common_name_check = true`
- **内存不足**：增大响应缓冲区或使用流式解析
- **请求超时**：LLM生成回复需1-5秒，timeout设30s以上
- **中文乱码**：确认文件UTF-8编码

### 任务10.2: System Prompt设计 (20分钟)

**好的System Prompt要素:**
1. **明确身份** -- "你是XXX，不是YYY"
2. **限定输出** -- 字数、语气、格式
3. **给出示例** -- 描述说话风格和常用词汇
4. **设置边界** -- 什么能说、什么不能说
5. **分层设计** -- 核心人设 + 可切换的模式

```c
// 不同模式的角色提示词
static const char *PROMPT_FUNNY =
    "你是小星，现在处于\"调皮模式\"。喜欢讲冷笑话，偶尔捉弄主人...";

static const char *PROMPT_TEACHER =
    "你是小星，现在处于\"老师模式\"。用通俗易懂的语言解释技术概念...";

static const char *PROMPT_COMPANION =
    "你是小星，现在处于\"陪伴模式\"。像知心朋友一样倾听和安慰...";
```

---

## 下午: 完整AI对话流水线 | Afternoon: Complete AI Dialogue Pipeline

### 任务10.3: 语音输入->ASR (30分钟)

```c
// ASR方案:
// 方案1: ESP-SR本地识别 (离线，适合命令词)
// 方案2: 云端ASR API（讯飞、百度、火山引擎）

// 简化版: 录音后发送云端ASR
static void voice_record_to_asr(void)
{
    ESP_LOGI(TAG, "开始录音 (3秒)...");
    
    // 录音3秒的PCM数据 (16kHz, 16bit, 单声道)
    int16_t *audio = malloc(16000 * 3 * sizeof(int16_t));
    size_t bytes_read;
    i2s_channel_read(rx_chan, audio, 16000 * 3 * sizeof(int16_t), 
                     &bytes_read, portMAX_DELAY);
    
    // 发送到云端ASR (此处简化)
    char *result = "你好呀，今天天气怎么样？";  // 模拟ASR结果
    ESP_LOGI(TAG, "ASR识别: %s", result);
    
    // 送入LLM对话
    process_ai_dialog(result);
    free(audio);
}
```

### 任务10.4: AI回复->TTS输出 (30分钟)

```c
// TTS方案:
// 1. 云端TTS API (讯飞、百度) -- 音质好，需网络
// 2. ESP-TTS本地合成 -- 离线，词汇受限
// 3. 预录音频文件 -- 简单可靠，适合系统提示

// TTS语音播报
static void text_to_speech(const char *text)
{
    ESP_LOGI(TAG, "TTS播报: %s", text);
    
    // 调用云端TTS或本地TTS
    // esp_tts_handle_t tts = esp_tts_create();
    // esp_tts_set_voice(tts, &esp_tts_voice_xiaole);
    // esp_tts_speak(tts, text);
    
    // 同时显示在屏幕上
    show_ai_reply_on_screen(text);
}

// 完整对话流水线
void process_ai_dialog(const char *user_input)
{
    char reply[512] = {0};
    
    // 1. 显示"思考中"
    set_status("思考中...");
    
    // 2. 调用LLM
    if (call_llm_api(user_input, reply, sizeof(reply)) != ESP_OK) {
        strcpy(reply, "呜...网络好像不太稳定，稍后再试吧~");
    }
    
    // 3. 显示+播报
    show_ai_reply_on_screen(reply);
    text_to_speech(reply);
    
    // 4. 根据回复调整表情
    adjust_expression(reply);
}
```

### 任务10.5: 多平台AI切换 (20分钟)

```c
// AI平台配置
typedef struct {
    const char *name;
    const char *url;
    const char *key;
    const char *model;
} ai_platform_t;

static ai_platform_t platforms[] = {
    {"DeepSeek", "https://api.deepseek.com/v1/chat/completions", 
     "sk-xxx", "deepseek-chat"},
    {"火山引擎", "https://ark.cn-beijing.volces.com/api/v3/chat/completions",
     "xxx", "doubao-lite"},
    {"阿里百炼", "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions",
     "xxx", "qwen-turbo"},
};

static int current_platform = 0;

void switch_ai_platform(int idx)
{
    if (idx >= 0 && idx < sizeof(platforms)/sizeof(platforms[0])) {
        current_platform = idx;
        ESP_LOGI(TAG, "切换到AI平台: %s", platforms[idx].name);
    }
}
```

---

## 今日作业 | Homework

### 必做题
1. 成功调用至少一个LLM API (DeepSeek推荐)，获取AI回复
2. 实现对话历史管理 (至少保留5轮对话)
3. 将AI回复显示在屏幕 + 通过喇叭TTS播报
4. 设计一个SparkBot的AI角色 (角色名、性格、口头禅)

### 挑战题
1. 实现多AI平台切换功能 (在屏幕菜单中选择)
2. 添加情感分析: 根据AI回复自动调整SparkBot表情动画
3. 实现"记忆"功能: SparkBot记住用户告诉它的信息(名字、喜好)

### 思考题
1. 为什么System Prompt对LLM对话质量如此重要？好的System Prompt应包含什么？
2. 对话历史越长LLM响应越慢越贵。如何在"记忆"和"成本"间平衡？
3. 如果把API Key直接写在固件代码中有什么安全风险？如何改进？

---

## 明日预告 | Tomorrow's Preview

明天是功能整合日！我们将把前面9天开发的所有功能整合为流畅的系统，添加小游戏、ESP-NOW同步、USB投屏等进阶功能，并进行系统稳定性测试。

Tomorrow is integration day! We'll integrate all features into one smooth system, add minigames, ESP-NOW sync, USB screencasting, and stress-test the system.

---

## 参考资源 | References

- [DeepSeek API 文档](https://platform.deepseek.com/api-docs)
- [阿里百炼 API 文档](https://help.aliyun.com/zh/model-studio/)
- [火山引擎豆包大模型](https://www.volcengine.com/docs/82379)
- [cJSON GitHub](https://github.com/DaveGamble/cJSON)
- [ESP-IDF HTTPS请求配置](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/protocols/esp_http_client.html)
- [ESP32接入AI大语言模型实战（B站搜索）](https://search.bilibili.com/all?keyword=ESP32%20AI%20%E5%A4%A7%E6%A8%A1%E5%9E%8B)

*最后更新：2026-05-27*
