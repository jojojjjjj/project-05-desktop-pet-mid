# ESP-SparkBot 单元测试指南
# ESP-SparkBot Unit Testing Guide

> 替代原来的 tests/test_basic.py
> Replaces the original tests/test_basic.py

---

## 概述 | Overview

ESP-SparkBot 使用 ESP-IDF 内置的 **Unity 测试框架** 进行单元测试和硬件集成测试。
ESP-SparkBot uses ESP-IDF's built-in **Unity test framework** for unit tests and hardware integration tests.

Unity 是一个轻量级的 C 语言单元测试框架，ESP-IDF 已将其集成到构建系统中。
Unity is a lightweight C unit test framework that ESP-IDF has integrated into its build system.

---

## 运行测试 | Running Tests

### 方法一: 在目标硬件上运行 (推荐)
### Method 1: Run on target hardware (recommended)

```bash
# 1. 编译并烧录测试固件
# Build and flash test firmware
cd esp_sparkbot/tests
idf.py set-target esp32s3
idf.py build
idf.py -p COM3 flash monitor

# 2. 在串口终端中输入 Unity 命令
# In the serial terminal, enter Unity commands:
test_all              # 运行所有测试 | Run all tests
test_state            # 运行状态机测试 | Run state machine tests
test_utils            # 运行工具函数测试 | Run utility tests
test_physics          # 运行物理模拟测试 | Run physics tests

# 3. 查看测试结果
# View test results
# PASSED: 测试通过
# FAILED: 测试失败 (会显示失败的断言详情)
```

### 方法二: 使用 ESP-IDF 的单元测试应用
### Method 2: Use ESP-IDF's unit test app

```bash
# ESP-IDF 自带一个单元测试应用程序
# ESP-IDF comes with a unit test application
cd $IDF_PATH/tools/unit-test-app

# 复制测试组件到测试应用
# Copy test components to test app
cp -r /path/to/esp_sparkbot/tests/* components/test/

# 编译并运行
idf.py build flash monitor
```

---

## 测试分类 | Test Categories

| 标签 (Tag) | 说明 (Description) |
|-----------|-------------------|
| `[state_machine]` | 状态机单元测试 | State machine unit tests |
| `[utils]` | 工具函数测试 | Utility function tests |
| `[physics]` | 物理模拟测试 | Physics simulation tests |
| `[integration]` | 集成测试 | Integration tests |

使用标签过滤运行特定测试:
Filter tests by tag:

```bash
# 只运行状态机测试
test_state
```

---

## 添加新测试 | Adding New Tests

在 `test_basic.c` 或新文件中使用 `TEST_CASE` 宏:

```c
#include "unity.h"

TEST_CASE("My Test: description", "[mytag]")
{
    // 准备 | Arrange
    int expected = 42;

    // 执行 | Act
    int actual = my_function();

    // 断言 | Assert
    TEST_ASSERT_EQUAL(expected, actual);
}
```

### 常用断言宏 | Common Assertion Macros

| 断言 (Assertion) | 说明 (Description) |
|---|---|
| `TEST_ASSERT(condition)` | 条件为真 | Condition is true |
| `TEST_ASSERT_TRUE(condition)` | 同 TEST_ASSERT |
| `TEST_ASSERT_FALSE(condition)` | 条件为假 | Condition is false |
| `TEST_ASSERT_EQUAL(expected, actual)` | 整数相等 | Integer equal |
| `TEST_ASSERT_EQUAL_STRING(expected, actual)` | 字符串相等 | String equal |
| `TEST_ASSERT_EQUAL_FLOAT(expected, actual)` | 浮点数相等 | Float equal |
| `TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)` | 浮点数在误差范围内 | Float within delta |
| `TEST_ASSERT_NOT_NULL(pointer)` | 指针非空 | Pointer not NULL |
| `TEST_ASSERT_NULL(pointer)` | 指针为空 | Pointer is NULL |
| `TEST_ASSERT_GREATER_THAN(threshold, actual)` | 大于 | Greater than |
| `TEST_ASSERT_LESS_THAN(threshold, actual)` | 小于 | Less than |

---

## 硬件集成测试 | Hardware Integration Tests

硬件相关的测试需要在连接了实际硬件的 ESP32-S3 上运行:

```c
TEST_CASE("Hardware: BMI270 responds", "[hardware][imu]")
{
    // 初始化 I2C
    sparkbot_i2c_init();

    // 初始化 IMU
    esp_err_t ret = touch_motion_imu_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    // 读取数据
    imu_data_t data;
    ret = touch_motion_read_imu(&data);
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    // 加速度计数据不应全为零
    TEST_ASSERT_TRUE(data.accel_x != 0 || data.accel_y != 0 || data.accel_z != 0);
}
```

---

## 测试文件映射 | Test File Mapping

| 原始 Python 文件 | 新 C 测试 |
|-----------------|----------|
| `tests/test_basic.py` → `test_TestStateMachine` | `test_basic.c` → `TEST_CASE("State Machine: ...")` |
| `tests/test_basic.py` → `test_TestAnimationManager` | (在显示任务中集成测试) |
| `tests/test_basic.py` → `test_TestPhysicsBody` | `test_basic.c` → `TEST_CASE("Physics: ...")` |
| `tests/test_basic.py` → `test_TestDialogueManager` | (在 AI 组件中集成测试) |
| `tests/test_basic.py` → `test_TestNeedsSystem` | `test_basic.c` → `TEST_CASE("State Machine: needs ...")` |
| `tests/test_basic.py` → `test_TestUtils` | `test_basic.c` → `TEST_CASE("Utils: ...")` |

---

## 持续集成建议 | CI Recommendations

```yaml
# .github/workflows/test.yml (GitHub Actions 示例)
name: ESP-SparkBot Tests
on: [push, pull_request]
jobs:
  build-and-test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build test firmware
        uses: espressif/esp-idf-ci-action@v1
        with:
          esp_idf_version: v5.3
          target: esp32s3
          path: tests
```
