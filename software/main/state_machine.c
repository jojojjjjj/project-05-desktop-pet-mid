/**
 * ================================================================
 * ESP-SparkBot -- 状态机实现
 * ESP-SparkBot -- State Machine Implementation
 * ================================================================
 *
 * 替代原来的 src/state_machine.py
 * Replaces the original src/state_machine.py
 *
 * 实现有限状态机 (FSM):
 *   - 状态转换表驱动
 *   - 事件队列处理
 *   - 需求系统模拟
 *
 * Implements finite state machine (FSM):
 *   - Transition table driven
 *   - Event queue processing
 *   - Needs system simulation
 *
 * 状态转换图 | State Transition Diagram:
 *
 *     INIT ──▶ IDLE ──────▶ LISTENING ──▶ SPEAKING ──▶ IDLE
 *               │                │
 *               │                ▼
 *               │             GAMING ──▶ IDLE
 *               │
 *               ├──▶ CHARGING ──────────▶ IDLE
 *               ├──▶ SLEEP ─────────────▶ IDLE
 *               └──▶ ERROR ─────────────▶ IDLE
 */

#include "state_machine.h"
#include "utils.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <string.h>

static const char *TAG = "state_machine";

/* ================================================================
 * 全局变量 | Global Variables
 * ================================================================ */

/* 当前状态 | Current state */
static sparkbot_state_t s_current_state = SPARKBOT_STATE_INIT;

/* 事件队列句柄 | Event queue handle */
static QueueHandle_t s_event_queue = NULL;

/* 需求系统实例 | Needs system instance */
static needs_system_t s_needs = {0};

/* 状态进入时间戳 (us) | State entry timestamp (us) */
static int64_t s_state_entry_time = 0;

/* 上一次需求更新的时间戳 | Last needs update timestamp */
static int64_t s_last_needs_update = 0;

/* ================================================================
 * 状态名称查询表 | State Name Lookup Table
 * ================================================================ */

static const char *s_state_names[SPARKBOT_STATE_MAX] = {
    [SPARKBOT_STATE_INIT]      = "INIT",
    [SPARKBOT_STATE_IDLE]      = "IDLE",
    [SPARKBOT_STATE_LISTENING] = "LISTENING",
    [SPARKBOT_STATE_SPEAKING]  = "SPEAKING",
    [SPARKBOT_STATE_GAMING]    = "GAMING",
    [SPARKBOT_STATE_CHARGING]  = "CHARGING",
    [SPARKBOT_STATE_SLEEP]     = "SLEEP",
    [SPARKBOT_STATE_ERROR]     = "ERROR",
};

/* ================================================================
 * 转换规则矩阵 | Transition Rule Matrix
 * ================================================================
 *
 * transition_rules[current_state][target_state] = 是否允许
 * transition_rules[current_state][target_state] = allowed?
 *
 * 转换规则说明 | Transition Rules:
 *   INIT  → IDLE  | 系统初始化完成后进入待机
 *   IDLE  → LISTENING, SPEAKING, GAMING, CHARGING, SLEEP, ERROR
 *   LISTENING → SPEAKING, IDLE  | 听到语音 → AI回复 / 超时 → 待机
 *   SPEAKING → IDLE, GAMING     | 说完话 → 待机 / 用户要求游戏
 *   GAMING → IDLE               | 游戏结束 → 待机
 *   CHARGING → IDLE             | 拔掉USB → 待机
 *   SLEEP → IDLE                | 被唤醒 → 待机
 *   ERROR → IDLE                | 错误恢复 → 待机
 */

static const bool s_transition_table[SPARKBOT_STATE_MAX][SPARKBOT_STATE_MAX] = {
    /*               TO: INIT IDLE LIST SPEAK GAME CHRG SLEEP ERR  */
    /* FROM: INIT */  {  0,   1,   0,   0,    0,   0,   0,   0  },
    /* FROM: IDLE */  {  0,   0,   1,   0,    1,   1,   1,   1  },
    /* FROM: LIST */  {  0,   1,   0,   1,    0,   0,   0,   0  },
    /* FROM: SPEAK */ {  0,   1,   0,   0,    1,   0,   0,   0  },
    /* FROM: GAME */  {  0,   1,   0,   0,    0,   0,   0,   0  },
    /* FROM: CHRG */  {  0,   1,   0,   0,    0,   0,   0,   0  },
    /* FROM: SLEEP */ {  0,   1,   0,   0,    0,   0,   0,   0  },
    /* FROM: ERR */   {  0,   1,   0,   0,    0,   0,   0,   0  },
};

/* ================================================================
 * 前向声明 | Forward Declarations
 * ================================================================ */
static void sm_enter_state(sparkbot_state_t new_state);
static void sm_exit_state(sparkbot_state_t old_state);
static void sm_update_needs(void);
static void sm_process_event(sm_event_t event, void *data);

/* ================================================================
 * 初始化 | Initialization
 * ================================================================ */

void sm_init(QueueHandle_t event_queue)
{
    s_event_queue = event_queue;

    /* 初始化需求系统 | Initialize needs system */
    s_needs.hunger    = 80.0f;
    s_needs.energy    = 100.0f;
    s_needs.happiness = 70.0f;
    s_needs.boredom   = 30.0f;

    s_needs.hunger_decay      = 0.5f;
    s_needs.energy_decay      = 0.3f;
    s_needs.happiness_decay   = 0.2f;
    s_needs.boredom_increase  = 0.4f;

    s_needs.hungry_threshold  = 30.0f;
    s_needs.sleepy_threshold  = 20.0f;
    s_needs.sad_threshold     = 25.0f;
    s_needs.bored_threshold   = 70.0f;

    s_state_entry_time  = esp_timer_get_time();
    s_last_needs_update = s_state_entry_time;

    /* 进入初始状态 | Enter initial state */
    sm_enter_state(SPARKBOT_STATE_IDLE);

    ESP_LOGI(TAG, "[状态机] 初始化完成, 状态: %s | Initialized, state: %s",
             sm_get_state_name());
}

/* ================================================================
 * 主更新循环 | Main Update Loop
 * ================================================================ */

void sm_update(void)
{
    /* 更新需求系统 | Update needs system */
    sm_update_needs();

    /* 处理事件队列中的事件 | Process events from queue */
    sm_event_t event;
    void *event_data = NULL;

    /* 非阻塞读取队列 (不阻塞状态机主循环) */
    /* Non-blocking queue read (don't block SM main loop) */
    while (xQueueReceive(s_event_queue, &event, 0) == pdTRUE) {
        sm_process_event(event, event_data);
    }

    /* 检查是否应该从 IDLE 转到 SLEEP */
    /* Check if should transition from IDLE to SLEEP */
    if (s_current_state == SPARKBOT_STATE_IDLE) {
        int64_t idle_duration = esp_timer_get_time() - s_state_entry_time;
        int64_t timeout_us = (int64_t)CONFIG_SPARKBOT_SLEEP_TIMEOUT * 1000;
        if (idle_duration >= timeout_us) {
            ESP_LOGI(TAG, "[状态机] 空闲超时, 进入睡眠 | Idle timeout, sleeping");
            sm_enter_state(SPARKBOT_STATE_SLEEP);
        }
    }
}

/* ================================================================
 * 事件处理 | Event Handling
 * ================================================================ */

void sm_handle_event(sm_event_t event, void *data)
{
    /* 将事件推入队列供 sm_update 处理 */
    /* Push event into queue for sm_update to process */
    if (s_event_queue != NULL) {
        xQueueSend(s_event_queue, &event, 0);
    }
}

static void sm_process_event(sm_event_t event, void *data)
{
    (void)data;  /* 大部分事件不携带数据 | Most events don't carry data */

    ESP_LOGD(TAG, "[状态机] 处理事件: %d (状态: %s) | Processing event: %d (state: %s)",
             event, sm_get_state_name());

    switch (event) {
        case SM_EVENT_TOUCH:
            /* 触摸事件 — 快速互动反馈 */
            /* Touch event — quick interaction feedback */
            sm_do_pet();
            ESP_LOGD(TAG, "[状态机] 被摸了一下 | Got petted");
            break;

        case SM_EVENT_DOUBLE_TAP:
            /* 双击 — 进入聆听模式, 准备AI对话 */
            /* Double tap — enter listening mode, prepare AI chat */
            sm_enter_state(SPARKBOT_STATE_LISTENING);
            break;

        case SM_EVENT_SHAKE:
            /* 摇晃 — 在待机时随机切换表情/动画 */
            /* Shake — randomly switch expression/animation in idle */
            if (s_current_state == SPARKBOT_STATE_IDLE) {
                ESP_LOGI(TAG, "[状态机] 摇晃! 切换表情 | Shake! Switch expression");
                /* 随机选择一种互动 | Randomly pick an interaction */
                uint8_t r = esp_random() % 3;
                if (r == 0) sm_do_feed();
                else if (r == 1) sm_do_play();
                else sm_do_pet();
            }
            break;

        case SM_EVENT_TILT:
            /* 倾斜 — 触发小游戏 */
            /* Tilt — trigger mini-game */
            if (s_current_state == SPARKBOT_STATE_IDLE ||
                s_current_state == SPARKBOT_STATE_SPEAKING) {
                sm_enter_state(SPARKBOT_STATE_GAMING);
            }
            break;

        case SM_EVENT_SPEECH_END:
            /* 语音识别完成 — 调用 AI */
            /* Speech recognition done — call AI */
            if (s_current_state == SPARKBOT_STATE_LISTENING) {
                sm_enter_state(SPARKBOT_STATE_SPEAKING);
            }
            break;

        case SM_EVENT_TTS_COMPLETE:
            /* TTS 播放完成 — 回到待机 */
            /* TTS done — back to idle */
            if (s_current_state == SPARKBOT_STATE_SPEAKING) {
                sm_enter_state(SPARKBOT_STATE_IDLE);
            }
            break;

        case SM_EVENT_GAME_COMPLETE:
            /* 游戏结束 — 回到待机 */
            /* Game over — back to idle */
            if (s_current_state == SPARKBOT_STATE_GAMING) {
                sm_enter_state(SPARKBOT_STATE_IDLE);
            }
            break;

        case SM_EVENT_CHARGING:
            /* USB 插入 — 进入充电状态 */
            /* USB plugged — enter charging state */
            sm_enter_state(SPARKBOT_STATE_CHARGING);
            break;

        case SM_EVENT_UNPLUGGED:
            /* USB 拔出 — 回到待机 */
            /* USB unplugged — back to idle */
            if (s_current_state == SPARKBOT_STATE_CHARGING) {
                sm_enter_state(SPARKBOT_STATE_IDLE);
            }
            break;

        case SM_EVENT_TIMEOUT:
            /* 空闲超时 — 进入睡眠 */
            /* Idle timeout — enter sleep */
            if (s_current_state == SPARKBOT_STATE_IDLE) {
                sm_enter_state(SPARKBOT_STATE_SLEEP);
            }
            break;

        case SM_EVENT_ERROR:
            /* 错误 — 进入错误状态 */
            /* Error — enter error state */
            sm_enter_state(SPARKBOT_STATE_ERROR);
            break;

        case SM_EVENT_ERROR_RESOLVED:
            /* 错误解决 — 回到待机 */
            /* Error resolved — back to idle */
            if (s_current_state == SPARKBOT_STATE_ERROR) {
                sm_enter_state(SPARKBOT_STATE_IDLE);
            }
            break;

        default:
            ESP_LOGW(TAG, "[状态机] 未处理的事件: %d | Unhandled event: %d", event);
            break;
    }
}

/* ================================================================
 * 状态转换 | State Transitions
 * ================================================================ */

void sm_request_transition(sparkbot_state_t target_state)
{
    /* 检查转换规则 | Check transition rules */
    if (target_state >= SPARKBOT_STATE_MAX) {
        ESP_LOGE(TAG, "[状态机] 无效的目标状态: %d | Invalid target state: %d",
                 target_state);
        return;
    }

    if (!s_transition_table[s_current_state][target_state]) {
        ESP_LOGW(TAG, "[状态机] 转换被拒绝: %s -> %s | Transition rejected: %s -> %s",
                 sm_get_state_name(), s_state_names[target_state]);
        return;
    }

    sm_enter_state(target_state);
}

void sm_force_state(sparkbot_state_t new_state)
{
    /* 强制转换 — 不检查规则表 */
    /* Force transition — don't check rule table */
    if (new_state >= SPARKBOT_STATE_MAX) {
        ESP_LOGE(TAG, "[状态机] 无效的目标状态: %d | Invalid target state: %d",
                 new_state);
        return;
    }

    sm_enter_state(new_state);
}

sparkbot_state_t sm_get_state(void)
{
    return s_current_state;
}

const char *sm_get_state_name(void)
{
    if (s_current_state < SPARKBOT_STATE_MAX) {
        return s_state_names[s_current_state];
    }
    return "UNKNOWN";
}

/* ================================================================
 * 内部: 状态进入与退出 | Internal: State Enter & Exit
 * ================================================================ */

static void sm_enter_state(sparkbot_state_t new_state)
{
    if (new_state == s_current_state) {
        return;  /* 已经是该状态 | Already in this state */
    }

    /* 退出旧状态 | Exit old state */
    sm_exit_state(s_current_state);

    /* 记录进入时间 | Record entry time */
    s_state_entry_time = esp_timer_get_time();

    /* 切换状态 | Switch state */
    sparkbot_state_t old_state = s_current_state;
    s_current_state = new_state;

    ESP_LOGI(TAG, "[状态机] 状态转换: %s -> %s | State transition: %s -> %s",
             s_state_names[old_state], s_state_names[new_state]);

    /* 状态进入逻辑 | State entry logic */
    switch (new_state) {
        case SPARKBOT_STATE_IDLE:
            ESP_LOGI(TAG, "[状态机] → 待机模式 | → Idle mode");
            /* 显示待机表情 | Show idle expression */
            break;

        case SPARKBOT_STATE_LISTENING:
            ESP_LOGI(TAG, "[状态机] → 聆听模式 | → Listening mode");
            /* 显示聆听动画 (耳朵动) | Show listening animation (ears wiggle) */
            /* 启动 VAD 检测 | Start VAD detection */
            break;

        case SPARKBOT_STATE_SPEAKING:
            ESP_LOGI(TAG, "[状态机] → 说话模式 | → Speaking mode");
            /* 显示说话动画 (嘴巴动) | Show speaking animation (mouth moves) */
            break;

        case SPARKBOT_STATE_GAMING:
            ESP_LOGI(TAG, "[状态机] → 游戏模式 | → Gaming mode");
            /* 启动游戏界面 | Start game UI */
            break;

        case SPARKBOT_STATE_CHARGING:
            ESP_LOGI(TAG, "[状态机] → 充电模式 | → Charging mode");
            break;

        case SPARKBOT_STATE_SLEEP:
            ESP_LOGI(TAG, "[状态机] → 睡眠模式 zZZ | → Sleep mode");
            /* 关闭显示背光, 降低 CPU 频率 | Turn off display backlight, lower CPU */
            break;

        case SPARKBOT_STATE_ERROR:
            ESP_LOGE(TAG, "[状态机] → 错误模式! | → Error mode!");
            /* 显示错误图标 | Show error icon */
            break;

        default:
            break;
    }
}

static void sm_exit_state(sparkbot_state_t old_state)
{
    /* 状态退出清理 | State exit cleanup */
    switch (old_state) {
        case SPARKBOT_STATE_SLEEP:
            ESP_LOGI(TAG, "[状态机] 退出睡眠模式 — 唤醒! | Exiting sleep — wake up!");
            /* 恢复显示背光 | Restore display backlight */
            break;

        case SPARKBOT_STATE_LISTENING:
            ESP_LOGI(TAG, "[状态机] 退出聆听模式 | Exiting listening mode");
            /* 停止录音 | Stop recording */
            break;

        case SPARKBOT_STATE_GAMING:
            ESP_LOGI(TAG, "[状态机] 退出游戏模式 | Exiting gaming mode");
            /* 清理游戏状态 | Clean up game state */
            break;

        default:
            break;
    }
}

/* ================================================================
 * 需求系统更新 | Needs System Update
 * ================================================================ */

static void sm_update_needs(void)
{
    int64_t now = esp_timer_get_time();
    float dt = (float)(now - s_last_needs_update) / 1000000.0f;  /* 转换为秒 | Convert to seconds */
    s_last_needs_update = now;

    /* 限制 dt 最大值 (防止长时间暂停后突变) */
    /* Clamp dt max (prevent sudden jumps after long pause) */
    if (dt > 5.0f) dt = 5.0f;
    if (dt < 0.0f) dt = 0.0f;

    /* 需求值随时间的自然变化 */
    /* Natural needs value change over time */
    if (s_current_state == SPARKBOT_STATE_SLEEP) {
        /* 睡眠时恢复体力, 其他缓慢衰减 */
        /* Recover energy during sleep, others decay slowly */
        s_needs.energy    = MIN(100.0f, s_needs.energy    + 2.0f * dt);
        s_needs.hunger    = MAX(0.0f,   s_needs.hunger    - 0.2f * dt);
        s_needs.happiness = MAX(0.0f,   s_needs.happiness - 0.1f * dt);
        s_needs.boredom   = MIN(100.0f, s_needs.boredom   + 0.1f * dt);
    } else {
        /* 正常活动时的需求衰减 */
        /* Normal activity needs decay */
        s_needs.hunger    = MAX(0.0f,   s_needs.hunger    - s_needs.hunger_decay * dt);
        s_needs.energy    = MAX(0.0f,   s_needs.energy    - s_needs.energy_decay * dt);
        s_needs.happiness = MAX(0.0f,   s_needs.happiness - s_needs.happiness_decay * dt);
        s_needs.boredom   = MIN(100.0f, s_needs.boredom   + s_needs.boredom_increase * dt);
    }

    /* 检查阈值 — 饥饿优先于困倦优先于无聊 */
    /* Check thresholds — hunger > sleepiness > boredom */
    /* (ESP_LOG 只在严重时输出, 避免刷屏) */
    /* (ESP_LOG only on severity, avoid spam) */
}

/* ================================================================
 * 互动操作 | Interaction Actions
 * ================================================================ */

void sm_do_feed(void)
{
    s_needs.hunger    = MIN(100.0f, s_needs.hunger + 30.0f);
    s_needs.happiness = MIN(100.0f, s_needs.happiness + 10.0f);
    ESP_LOGI(TAG, "[互动] 喂食! 饱食度=%.0f | Feed! Hunger=%.0f", s_needs.hunger);
}

void sm_do_play(void)
{
    s_needs.happiness = MIN(100.0f, s_needs.happiness + 15.0f);
    s_needs.boredom   = MAX(0.0f,   s_needs.boredom - 20.0f);
    s_needs.energy    = MAX(0.0f,   s_needs.energy - 5.0f);
    ESP_LOGI(TAG, "[互动] 玩耍! 快乐值=%.0f | Play! Happiness=%.0f", s_needs.happiness);
}

void sm_do_pet(void)
{
    s_needs.happiness = MIN(100.0f, s_needs.happiness + 10.0f);
    ESP_LOGI(TAG, "[互动] 抚摸! 快乐值=%.0f | Pet! Happiness=%.0f", s_needs.happiness);
}

const needs_system_t *sm_get_needs(void)
{
    return &s_needs;
}
