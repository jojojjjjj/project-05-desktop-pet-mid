/**
 * ================================================================
 * ESP-SparkBot -- 单元测试
 * ESP-SparkBot -- Unit Tests
 * ================================================================
 *
 * 替代原来的 tests/test_basic.py
 * Replaces the original tests/test_basic.py
 *
 * 使用 ESP-IDF Unity 测试框架。
 * Uses ESP-IDF Unity test framework.
 *
 * 运行方式 | How to run:
 *   cd esp_sparkbot
 *   idf.py build
 *   idf.py -p COM3 flash monitor
 *
 *   在串口终端中输入 Unity 测试命令:
 *   In serial terminal, enter Unity test commands:
 *     test_all          — 运行所有测试 | Run all tests
 *     test_state        — 运行状态机测试 | Run state machine tests
 *     test_utils        — 运行工具函数测试 | Run utility tests
 *     test_touch        — 运行触摸读取测试 | Run touch read test
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "unity.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/* 引入被测试的模块头文件 (C 文件直接包含) */
/* Include the headers of modules under test */
#include "state_machine.h"
#include "utils.h"

static const char *TAG = "test";

/* ================================================================
 * 测试辅助函数 | Test Helper Functions
 * ================================================================ */

/**
 * @brief 每个测试用例执行前调用
 * Called before each test case
 */
void setUp(void)
{
    /* 可选的初始化 | Optional initialization */
}

/**
 * @brief 每个测试用例执行后调用
 * Called after each test case
 */
void tearDown(void)
{
    /* 可选的清理 | Optional cleanup */
}

/* ================================================================
 * 状态机测试 | State Machine Tests
 * ================================================================ */

/**
 * @brief 测试: 状态机初始化后默认为 IDLE
 * Test: State machine defaults to IDLE after init
 */
TEST_CASE("State Machine: init defaults to IDLE", "[state_machine]")
{
    QueueHandle_t test_queue = xQueueCreate(10, sizeof(sm_event_t));
    TEST_ASSERT_NOT_NULL(test_queue);

    sm_init(test_queue);

    TEST_ASSERT_EQUAL(SPARKBOT_STATE_IDLE, sm_get_state());
    TEST_ASSERT_EQUAL_STRING("IDLE", sm_get_state_name());

    vQueueDelete(test_queue);
}

/**
 * @brief 测试: 状态强制切换
 * Test: Force state transition bypasses rules
 */
TEST_CASE("State Machine: force state transition", "[state_machine]")
{
    QueueHandle_t test_queue = xQueueCreate(10, sizeof(sm_event_t));
    sm_init(test_queue);

    /* 强制进入 GAMING 状态 | Force into GAMING state */
    sm_force_state(SPARKBOT_STATE_GAMING);
    TEST_ASSERT_EQUAL(SPARKBOT_STATE_GAMING, sm_get_state());

    /* 强制进入 ERROR 状态 | Force into ERROR state */
    sm_force_state(SPARKBOT_STATE_ERROR);
    TEST_ASSERT_EQUAL(SPARKBOT_STATE_ERROR, sm_get_state());

    /* 恢复 IDLE | Restore IDLE */
    sm_force_state(SPARKBOT_STATE_IDLE);
    TEST_ASSERT_EQUAL(SPARKBOT_STATE_IDLE, sm_get_state());

    vQueueDelete(test_queue);
}

/**
 * @brief 测试: 需求系统初始化值
 * Test: Needs system initial values
 */
TEST_CASE("State Machine: needs system initial values", "[state_machine]")
{
    QueueHandle_t test_queue = xQueueCreate(10, sizeof(sm_event_t));
    sm_init(test_queue);

    const needs_system_t *needs = sm_get_needs();
    TEST_ASSERT_NOT_NULL(needs);

    TEST_ASSERT_EQUAL_FLOAT(80.0f, needs->hunger);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, needs->energy);
    TEST_ASSERT_EQUAL_FLOAT(70.0f, needs->happiness);
    TEST_ASSERT_EQUAL_FLOAT(30.0f, needs->boredom);

    vQueueDelete(test_queue);
}

/**
 * @brief 测试: 喂食操作
 * Test: Feed interaction
 */
TEST_CASE("State Machine: feed increases hunger", "[state_machine]")
{
    QueueHandle_t test_queue = xQueueCreate(10, sizeof(sm_event_t));
    sm_init(test_queue);

    const needs_system_t *needs = sm_get_needs();
    float before = needs->hunger;

    sm_do_feed();

    /* 饱食度应该增加 | Hunger should increase */
    TEST_ASSERT_GREATER_THAN_FLOAT(before, needs->hunger);

    vQueueDelete(test_queue);
}

/**
 * @brief 测试: 玩耍操作
 * Test: Play interaction
 */
TEST_CASE("State Machine: play interaction", "[state_machine]")
{
    QueueHandle_t test_queue = xQueueCreate(10, sizeof(sm_event_t));
    sm_init(test_queue);

    const needs_system_t *needs = sm_get_needs();
    float happy_before = needs->happiness;
    float bored_before = needs->boredom;

    sm_do_play();

    /* 快乐值应该增加 | Happiness should increase */
    TEST_ASSERT_GREATER_THAN_FLOAT(happy_before, needs->happiness);
    /* 无聊值应该减少 | Boredom should decrease */
    TEST_ASSERT_LESS_THAN_FLOAT(bored_before, needs->boredom);

    vQueueDelete(test_queue);
}

/* ================================================================
 * 工具函数测试 | Utility Function Tests
 * ================================================================ */

/**
 * @brief 测试: CLAMP 宏
 * Test: CLAMP macro
 */
TEST_CASE("Utils: clamp macro", "[utils]")
{
    TEST_ASSERT_EQUAL(5, CLAMP(5, 0, 10));
    TEST_ASSERT_EQUAL(0, CLAMP(-5, 0, 10));
    TEST_ASSERT_EQUAL(10, CLAMP(15, 0, 10));
    TEST_ASSERT_EQUAL(0, CLAMP(0, 0, 10));
    TEST_ASSERT_EQUAL(10, CLAMP(10, 0, 10));
}

/**
 * @brief 测试: MIN / MAX 宏
 * Test: MIN / MAX macros
 */
TEST_CASE("Utils: min/max macros", "[utils]")
{
    TEST_ASSERT_EQUAL(3, MIN(3, 5));
    TEST_ASSERT_EQUAL(5, MAX(3, 5));
    TEST_ASSERT_EQUAL(-1, MIN(-1, 0));
    TEST_ASSERT_EQUAL(0, MAX(-1, 0));
}

/**
 * @brief 测试: 线性插值
 * Test: Linear interpolation
 */
TEST_CASE("Utils: lerp function", "[utils]")
{
    TEST_ASSERT_EQUAL_FLOAT(5.0f, lerp(0.0f, 10.0f, 0.5f));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, lerp(0.0f, 10.0f, 0.0f));
    TEST_ASSERT_EQUAL_FLOAT(10.0f, lerp(0.0f, 10.0f, 1.0f));
    TEST_ASSERT_EQUAL_FLOAT(2.5f, lerp(0.0f, 10.0f, 0.25f));
}

/**
 * @brief 测试: 两点距离
 * Test: Distance between two points
 */
TEST_CASE("Utils: distance_2d function", "[utils]")
{
    /* 3-4-5 三角形 | 3-4-5 triangle */
    float d = distance_2d(0.0f, 0.0f, 3.0f, 4.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.0f, d);

    /* 相同点距离为 0 | Same point distance = 0 */
    d = distance_2d(10.0f, 10.0f, 10.0f, 10.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, d);

    /* 负坐标 | Negative coordinates */
    d = distance_2d(-1.0f, -1.0f, 1.0f, 1.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.828f, d);  /* sqrt(8) ≈ 2.828 */
}

/**
 * @brief 测试: 随机数范围
 * Test: Random number range
 */
TEST_CASE("Utils: random_range bounded", "[utils]")
{
    for (int i = 0; i < 100; i++) {
        int32_t val = random_range(0, 10);
        TEST_ASSERT_GREATER_OR_EQUAL(0, val);
        TEST_ASSERT_LESS_OR_EQUAL(10, val);
    }

    for (int i = 0; i < 100; i++) {
        int32_t val = random_range(-5, 5);
        TEST_ASSERT_GREATER_OR_EQUAL(-5, val);
        TEST_ASSERT_LESS_OR_EQUAL(5, val);
    }
}

/**
 * @brief 测试: JSON 字符串提取
 * Test: JSON string extraction
 */
TEST_CASE("Utils: json_extract_string", "[utils]")
{
    const char *json = "{\"name\":\"SparkBot\",\"version\":\"1.0.0\"}";
    char value[64];

    esp_err_t ret = json_extract_string(json, "name", value, sizeof(value));
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL_STRING("SparkBot", value);

    ret = json_extract_string(json, "version", value, sizeof(value));
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL_STRING("1.0.0", value);

    /* 不存在的键应返回失败 | Non-existent key should fail */
    ret = json_extract_string(json, "nonexistent", value, sizeof(value));
    TEST_ASSERT_EQUAL(ESP_FAIL, ret);
}

/**
 * @brief 测试: 安全字符串拼接
 * Test: Safe string concatenation
 */
TEST_CASE("Utils: str_concat_safe", "[utils]")
{
    char buf[20];

    strcpy(buf, "Hello");
    str_concat_safe(buf, " World", sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("Hello World", buf);

    /* 测试缓冲区溢出防护 | Test buffer overflow protection */
    strcpy(buf, "Hello");
    str_concat_safe(buf, " This is a very long string that exceeds the buffer",
                    sizeof(buf));
    /* 应该不会崩溃, 并且以 null 结尾 | Should not crash, null-terminated */
    TEST_ASSERT_TRUE(strlen(buf) < sizeof(buf));
}

/**
 * @brief 测试: 字符串修剪
 * Test: String trim
 */
TEST_CASE("Utils: str_trim", "[utils]")
{
    char buf1[32] = "  hello  ";
    str_trim(buf1);
    TEST_ASSERT_EQUAL_STRING("hello", buf1);

    char buf2[32] = "no spaces";
    str_trim(buf2);
    TEST_ASSERT_EQUAL_STRING("no spaces", buf2);

    char buf3[32] = "";
    str_trim(buf3);
    TEST_ASSERT_EQUAL_STRING("", buf3);
}

/**
 * @brief 测试: 时间格式化
 * Test: Time formatting
 */
TEST_CASE("Utils: format_time_delta", "[utils]")
{
    char buf[64];

    format_time_delta(30, buf, sizeof(buf));
    TEST_ASSERT_TRUE(strstr(buf, "30秒") != NULL || strstr(buf, "30s") != NULL);

    format_time_delta(120, buf, sizeof(buf));
    TEST_ASSERT_TRUE(strstr(buf, "2分钟") != NULL || strstr(buf, "2min") != NULL);

    format_time_delta(7200, buf, sizeof(buf));
    TEST_ASSERT_TRUE(strstr(buf, "2小时") != NULL || strstr(buf, "2hr") != NULL);
}

/* ================================================================
 * 物理模拟测试 (如果组件已链接) | Physics Simulation Tests
 * ================================================================
 */

/**
 * @brief 测试: 物理体初始化
 * Test: Physics body init
 */
TEST_CASE("Physics: body init", "[physics]")
{
    physics_body_t body;
    physics_body_init(&body, 100.0f, 200.0f);

    TEST_ASSERT_EQUAL_FLOAT(100.0f, body.x);
    TEST_ASSERT_EQUAL_FLOAT(200.0f, body.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, body.vx);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, body.vy);
    TEST_ASSERT_EQUAL_FLOAT(0.92f, body.friction);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, body.bounce);
    TEST_ASSERT_FALSE(body.use_gravity);
}

/**
 * @brief 测试: 物理体抛掷
 * Test: Physics body fling
 */
TEST_CASE("Physics: body fling", "[physics]")
{
    physics_body_t body;
    physics_body_init(&body, 500.0f, 300.0f);
    body.friction = 1.0f;  /* 无摩擦 | No friction */

    physics_body_fling(&body, 10.0f, 5.0f);

    TEST_ASSERT_TRUE(physics_body_is_moving(&body));
    TEST_ASSERT_EQUAL_FLOAT(10.0f, body.vx);
    TEST_ASSERT_EQUAL_FLOAT(5.0f, body.vy);
}

/**
 * @brief 测试: 物理体停止
 * Test: Physics body stop
 */
TEST_CASE("Physics: body stop", "[physics]")
{
    physics_body_t body;
    physics_body_init(&body, 0.0f, 0.0f);
    physics_body_fling(&body, 10.0f, 10.0f);
    physics_body_stop(&body);

    TEST_ASSERT_FALSE(physics_body_is_moving(&body));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, body.vx);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, body.vy);
}

/**
 * @brief 测试: 物理体边界碰撞 (左边界)
 * Test: Physics body boundary collision (left)
 */
TEST_CASE("Physics: body boundary left", "[physics]")
{
    physics_body_t body;
    physics_body_init(&body, -10.0f, 100.0f);
    body.vx = -5.0f;
    body.bounce = 0.5f;
    body.friction = 1.0f;

    physics_body_update(&body, 800.0f, 600.0f, 200.0f, 200.0f);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, body.x);  /* 不应超出左边界 | Should not exceed left */
    TEST_ASSERT_TRUE(body.vx > 0.0f);        /* 速度应反向 | Velocity should reverse */
}

/* ================================================================
 * 集成测试 | Integration Tests
 * ================================================================ */

/**
 * @brief 测试: 状态机事件处理
 * Test: State machine event handling
 */
TEST_CASE("Integration: state machine event processing", "[integration]")
{
    QueueHandle_t test_queue = xQueueCreate(10, sizeof(sm_event_t));
    sm_init(test_queue);

    /* 发送双击事件 | Send double-tap event */
    sm_handle_event(SM_EVENT_DOUBLE_TAP, NULL);

    /* 给 FreeRTOS 一些时间处理 | Give FreeRTOS some time to process */
    vTaskDelay(pdMS_TO_TICKS(100));

    /* 应该进入 LISTENING 状态 | Should enter LISTENING state */
    TEST_ASSERT_EQUAL(SPARKBOT_STATE_LISTENING, sm_get_state());

    /* 发送语音结束事件 | Send speech end event */
    sm_handle_event(SM_EVENT_SPEECH_END, NULL);
    vTaskDelay(pdMS_TO_TICKS(100));

    /* 应该进入 SPEAKING 状态 | Should enter SPEAKING state */
    TEST_ASSERT_EQUAL(SPARKBOT_STATE_SPEAKING, sm_get_state());

    /* 发送 TTS 完成事件 | Send TTS complete event */
    sm_handle_event(SM_EVENT_TTS_COMPLETE, NULL);
    vTaskDelay(pdMS_TO_TICKS(100));

    /* 应该回到 IDLE 状态 | Should return to IDLE state */
    TEST_ASSERT_EQUAL(SPARKBOT_STATE_IDLE, sm_get_state());

    vQueueDelete(test_queue);
}

/**
 * @brief 测试: 堆内存基本分配
 * Test: Basic heap memory allocation
 */
TEST_CASE("Integration: heap memory allocation", "[integration]")
{
    size_t before = esp_get_free_heap_size();

    /* 分配一些内存 | Allocate some memory */
    void *ptr1 = malloc(1024);
    TEST_ASSERT_NOT_NULL(ptr1);

    void *ptr2 = malloc(2048);
    TEST_ASSERT_NOT_NULL(ptr2);

    /* 释放 | Free */
    free(ptr1);
    free(ptr2);

    size_t after = esp_get_free_heap_size();
    ESP_LOGI(TAG, "[测试] 堆内存: 分配前=%u 分配后=%u | Heap: before=%u after=%u",
             before, after);

    /* 分配前后的可用内存应该接近 (允许微小差异) */
    /* Available memory should be close before and after (small variance allowed) */
    TEST_ASSERT_INT_WITHIN(256, before, after);
}

/* ================================================================
 * 主测试入口 | Main Test Entry
 * ================================================================ */

/**
 * @brief Unity 测试框架的主入口 (ESP-IDF 自动调用)
 * Main entry for Unity test framework (auto-called by ESP-IDF)
 *
 * 在串口终端中运行测试:
 * Run tests in serial terminal:
 *   test_all
 */
void app_main(void)
{
    ESP_LOGI(TAG, "╔══════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║   ESP-SparkBot Unity 测试套件           ║");
    ESP_LOGI(TAG, "║   ESP-SparkBot Unity Test Suite          ║");
    ESP_LOGI(TAG, "╚══════════════════════════════════════════╝");
    ESP_LOGI(TAG, "[测试] 请输入 Unity 命令 | Enter Unity command:");
    ESP_LOGI(TAG, "  test_all     — 运行所有测试 | Run all tests");
    ESP_LOGI(TAG, "  test_state   — 状态机测试 | State machine tests");
    ESP_LOGI(TAG, "  test_utils   — 工具函数测试 | Utility tests");
    ESP_LOGI(TAG, "  test_physics — 物理模拟测试 | Physics tests");

    /* 启动 Unity 测试 */
    unity_run_all_tests();

    /* 正常退出 (ESP-IDF 会重启) */
    /* Normal exit (ESP-IDF will restart) */
}
