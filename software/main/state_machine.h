/**
 * ================================================================
 * ESP-SparkBot -- 状态机系统
 * ESP-SparkBot -- State Machine System
 * ================================================================
 *
 * 替代原来的 src/state_machine.py
 * Replaces the original src/state_machine.py
 *
 * 功能 (Functions):
 *   - 有限状态机 (FSM) C 语言实现
 *   - 6 个核心状态: IDLE, LISTENING, SPEAKING, GAMING, CHARGING, ERROR
 *   - 状态转换规则矩阵
 *   - 事件驱动架构
 *
 * 有限状态机 (Finite State Machine) C implementation.
 * 6 core states with transition rules and event-driven architecture.
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * 状态枚举 | State Enumeration
 * ================================================================ */

/**
 * @brief SparkBot 行为状态定义
 * SparkBot Behavior State Definitions
 *
 * 每个状态代表宠物的一个核心行为模式。
 * Each state represents a core behavior mode of the pet.
 */
typedef enum {
    SPARKBOT_STATE_INIT      = 0,  /* 系统初始化 | System initializing */
    SPARKBOT_STATE_IDLE      = 1,  /* 待机状态 | Idle — 显示默认表情, 等待交互 */
    SPARKBOT_STATE_LISTENING = 2,  /* 聆听状态 | Listening — 录音中, 等待语音结束 */
    SPARKBOT_STATE_SPEAKING  = 3,  /* 说话状态 | Speaking — AI回复, TTS播放中 */
    SPARKBOT_STATE_GAMING    = 4,  /* 游戏状态 | Gaming — 小游戏进行中 */
    SPARKBOT_STATE_CHARGING  = 5,  /* 充电状态 | Charging — USB充电, 显示电量 */
    SPARKBOT_STATE_SLEEP     = 6,  /* 睡眠状态 | Sleep — 空闲超时, 进入低功耗 */
    SPARKBOT_STATE_ERROR     = 7,  /* 错误状态 | Error — 发生错误, 显示故障信息 */
    SPARKBOT_STATE_MAX       = 8   /* 状态总数 (用于数组边界) | Total states (for array bounds) */
} sparkbot_state_t;

/* ================================================================
 * 状态机事件枚举 | State Machine Event Enumeration
 * ================================================================ */

/**
 * @brief 状态机内部事件定义
 * State Machine Internal Event Definitions
 */
typedef enum {
    SM_EVENT_NONE           = 0,  /* 无事件 | No event */
    SM_EVENT_TOUCH          = 1,  /* 触摸事件 | Touch event */
    SM_EVENT_DOUBLE_TAP     = 2,  /* 双击手势 | Double-tap gesture */
    SM_EVENT_SHAKE          = 3,  /* 摇晃手势 | Shake gesture */
    SM_EVENT_TILT           = 4,  /* 倾斜手势 | Tilt gesture */
    SM_EVENT_BUTTON_PRESS   = 5,  /* 按键按下 | Button press */
    SM_EVENT_SPEECH_START   = 6,  /* 语音识别开始 | Speech recognition start */
    SM_EVENT_SPEECH_END     = 7,  /* 语音识别结束 | Speech recognition end */
    SM_EVENT_AI_RESPONSE    = 8,  /* AI 回复到达 | AI response arrived */
    SM_EVENT_TTS_COMPLETE   = 9,  /* TTS 播放完成 | TTS playback complete */
    SM_EVENT_GAME_REQUEST   = 10, /* 游戏请求 | Game requested */
    SM_EVENT_GAME_COMPLETE  = 11, /* 游戏完成 | Game complete */
    SM_EVENT_CHARGING       = 12, /* USB 插入充电 | USB plugged for charging */
    SM_EVENT_UNPLUGGED      = 13, /* USB 拔出 | USB unplugged */
    SM_EVENT_LOW_BATTERY    = 14, /* 电量过低 | Battery critically low */
    SM_EVENT_TIMEOUT        = 15, /* 超时 (转睡眠) | Timeout (go to sleep) */
    SM_EVENT_ERROR          = 16, /* 错误发生 | Error occurred */
    SM_EVENT_ERROR_RESOLVED = 17, /* 错误解决 | Error resolved */
} sm_event_t;

/* ================================================================
 * 需求系统结构 | Needs System Structure
 * ================================================================ */

/**
 * @brief 宠物需求系统
 * Pet Needs System
 *
 * 管理宠物的虚拟需求值，类似电子宠物。
 * Manages the pet's virtual needs, similar to a Tamagotchi.
 */
typedef struct {
    float hunger;      /* 饱食度 (0-100, 越低越饿) | Hunger (0-100, lower = hungrier) */
    float energy;      /* 体力值 (0-100, 越低越累) | Energy (0-100, lower = more tired) */
    float happiness;   /* 快乐值 (0-100, 越低越不开心) | Happiness (0-100, lower = sadder) */
    float boredom;     /* 无聊值 (0-100, 越高越无聊) | Boredom (0-100, higher = more bored) */

    /* 衰减速率 (每秒) | Decay rates (per second) */
    float hunger_decay;
    float energy_decay;
    float happiness_decay;
    float boredom_increase;

    /* 阈值 | Thresholds */
    float hungry_threshold;
    float sleepy_threshold;
    float sad_threshold;
    float bored_threshold;
} needs_system_t;

/* ================================================================
 * 状态表条目 | State Table Entry
 * ================================================================ */

/**
 * @brief 单个状态的行为函数表
 * Function table for a single state's behavior
 */
typedef struct {
    void (*on_enter)(void);    /* 进入状态时调用 | Called when entering state */
    void (*on_exit)(void);     /* 退出状态时调用 | Called when exiting state */
    void (*on_update)(void);   /* 每帧更新调用 | Called every frame */
    bool (*can_transition)(sm_event_t event); /* 是否接受该事件导致的转换 */
} sm_state_ops_t;

/* ================================================================
 * 公共 API | Public API
 * ================================================================ */

/**
 * @brief 初始化状态机
 * Initialize State Machine
 *
 * @param event_queue 事件队列句柄，状态机会向此队列发送事件
 *                    Event queue handle, SM sends events here
 */
void sm_init(QueueHandle_t event_queue);

/**
 * @brief 状态机主更新循环 (每 50ms 调用)
 * State Machine Main Update Loop (called every 50ms)
 */
void sm_update(void);

/**
 * @brief 向状态机发送事件
 * Send an event to the state machine
 *
 * @param event 事件类型 | Event type
 * @param data  事件附加数据 | Event additional data
 */
void sm_handle_event(sm_event_t event, void *data);

/**
 * @brief 请求状态转换 (可能被拒绝, 取决于转换规则)
 * Request a state transition (may be rejected based on rules)
 *
 * @param target_state 目标状态 | Target state
 */
void sm_request_transition(sparkbot_state_t target_state);

/**
 * @brief 强制状态转换 (绕过所有规则)
 * Force a state transition (bypasses all rules)
 *
 * @param new_state 新状态 | New state
 */
void sm_force_state(sparkbot_state_t new_state);

/**
 * @brief 获取当前状态
 * Get current state
 *
 * @return 当前状态枚举值 | Current state enum value
 */
sparkbot_state_t sm_get_state(void);

/**
 * @brief 获取当前状态的名称字符串
 * Get the name string of the current state
 *
 * @return 状态名称 (如 "IDLE") | State name (e.g. "IDLE")
 */
const char *sm_get_state_name(void);

/**
 * @brief 获取需求系统指针 (只读)
 * Get needs system pointer (read-only)
 *
 * @return 需求系统指针 | Needs system pointer
 */
const needs_system_t *sm_get_needs(void);

/**
 * @brief 喂食互动 (增加饱食度)
 * Feed interaction (increase hunger)
 */
void sm_do_feed(void);

/**
 * @brief 玩耍互动 (增加快乐值)
 * Play interaction (increase happiness)
 */
void sm_do_play(void);

/**
 * @brief 抚摸互动 (增加快乐值)
 * Pet interaction (increase happiness)
 */
void sm_do_pet(void);

#ifdef __cplusplus
}
#endif

#endif /* STATE_MACHINE_H */
