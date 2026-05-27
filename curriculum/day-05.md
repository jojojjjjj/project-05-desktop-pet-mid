# Day 05: GPIO编程与传感器驱动 | GPIO Programming & Sensor Drivers

> **今日目标:**
> - 掌握ESP-IDF的GPIO编程方法（输出、输入、中断）
> - 学习I2C总线协议，读取BMI270加速度计/陀螺仪数据
> - 使用串口输出传感器数据到PC监视器
> - 编程控制ESP32-S3触摸按键
> - 理解FreeRTOS任务间通信（队列）
>
> **产出:** 通过GPIO控制LED和触摸按键，通过I2C成功读取BMI270传感器数据并在串口打印，构建传感器数据采集系统

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间活动 | 前日焊接PCB上电测试，检查硬件连接 |
| 09:15 - 10:30 | 知识讲解 | GPIO编程模型、中断处理、I2C总线协议原理 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | LED闪烁控制、按键读取与消抖、I2C初始化 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | BMI270数据读取、触摸按键编程 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | FreeRTOS任务间通信、综合传感器采集系统 |
| 16:30 - 17:00 | 总结分享 | 传感器数据可视化展示、代码Review |

---

## 上午: GPIO基础与I2C总线 | Morning: GPIO Basics & I2C Bus

### 为什么要学这个? | Why Learn This?

GPIO（General Purpose Input/Output）是嵌入式系统最基本、最核心的概念。一切外设控制——点亮LED、检测按键、发送信号——都从GPIO开始。I2C则是嵌入式系统中最常用的通信协议之一，传感器、显示屏、触摸控制器都通过它和主控对话。

GPIO is the most fundamental concept in embedded systems. All peripheral control -- lighting LEDs, detecting buttons, sending signals -- starts with GPIO. I2C is one of the most common communication protocols, used by sensors, displays, and touch controllers to talk to the main MCU.

### 任务5.1: GPIO输出 -- 控制LED闪烁 (30分钟)

**步骤:**

创建项目文件 `main/gpio_blink.c`：

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "GPIO_BLINK";

// SparkBot主板LED引脚定义（根据实际原理图）
#define LED_GPIO        GPIO_NUM_48    // 主板LED
#define BOOT_BUTTON     GPIO_NUM_0     // BOOT按键

void app_main(void)
{
    ESP_LOGI(TAG, "=== SparkBot GPIO LED 演示 ===");

    // ---- GPIO输出配置 ----
    gpio_reset_pin(LED_GPIO);                       // 复位引脚到默认状态
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT); // 设置为输出模式
    gpio_set_level(LED_GPIO, 0);                    // 初始状态: LED关闭

    // ---- GPIO输入配置（用于触发LED） ----
    gpio_reset_pin(BOOT_BUTTON);
    gpio_set_direction(BOOT_BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BOOT_BUTTON, GPIO_PULLUP_ONLY);  // 内部上拉

    int led_state = 0;
    while (1) {
        // 简单的LED闪烁
        led_state = !led_state;
        gpio_set_level(LED_GPIO, led_state);
        ESP_LOGI(TAG, "LED %s", led_state ? "ON  ●" : "OFF ○");
        vTaskDelay(500 / portTICK_PERIOD_MS);  // 每500ms翻转一次
    }
}
```

**gpio_set_direction 的三种模式:**
| 模式 | 说明 | 使用场景 |
|------|------|----------|
| `GPIO_MODE_OUTPUT` | 推挽输出 | LED、继电器等需要驱动电流的外设 |
| `GPIO_MODE_INPUT` | 高阻输入 | 按键、传感器数字信号输入 |
| `GPIO_MODE_OUTPUT_OD` | 开漏输出 | I2C总线、需要电平转换的场景 |

**预期结果:**
- 主板LED以1Hz频率闪烁（亮500ms，灭500ms）
- 串口每500ms打印一次状态信息

**常见问题:**
- **LED不亮**：检查GPIO引脚号是否正确（参考原理图），部分GPIO被用作PSRAM不能当普通GPIO
- **编译错误 undefined reference to gpio_xxx**：检查CMakeLists.txt源文件名是否一致

### 任务5.2: GPIO输入 -- 读取按键状态 (30分钟)

按键输入需要处理**抖动**问题。机械按键在按下和释放的瞬间会产生几十毫秒的抖动信号，必须用软件或硬件消除。

Mechanical buttons produce bouncing signals for tens of milliseconds during press and release, which must be eliminated by software or hardware.

```c
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "BUTTON";

#define BUTTON_GPIO     GPIO_NUM_0
#define DEBOUNCE_MS     50     // 消抖时间（毫秒）

// 按键消抖读取 -- 返回true表示按键确实被按下
static bool button_read_debounced(void)
{
    // 第一次读取
    if (gpio_get_level(BUTTON_GPIO) != 0) {
        return false;  // 未按下（上拉，高电平）
    }

    // 延时去抖
    vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));

    // 再次确认
    if (gpio_get_level(BUTTON_GPIO) != 0) {
        return false;  // 是抖动，不是真的按下
    }
    return true;  // 确认按下
}

// ---- 中断方式检测按键（替代轮询） ----
static void IRAM_ATTR button_isr_handler(void *arg)
{
    // 中断服务例程：只做最少的事情，通知任务处理
    // 实际的消抖和业务逻辑在任务中完成
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // ... 通过任务通知或队列唤醒处理任务
}

void app_main(void)
{
    // 配置按键GPIO
    gpio_reset_pin(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);

    // ---- 方式A: 轮询方式 ----
    ESP_LOGI(TAG, "使用轮询方式检测按键...");
    int press_count = 0;
    while (1) {
        if (button_read_debounced()) {
            press_count++;
            ESP_LOGI(TAG, "按键按下! 计数: %d", press_count);

            // 等待按键释放（避免重复计数）
            while (gpio_get_level(BUTTON_GPIO) == 0) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));  // 释放消抖
        }
        vTaskDelay(pdMS_TO_TICKS(10));  // 10ms轮询间隔
    }
}
```

**中断 vs 轮询对比:**
| 方式 | 优点 | 缺点 | 适用场景 |
|------|------|------|----------|
| 轮询 | 实现简单，行为可预测 | 占用CPU、有响应延迟 | 低速按键、教学项目 |
| 中断 | 即时响应、CPU效率高 | 代码复杂、需在ISR中注意限制 | 编码器、高速信号 |

**预期结果:**
- 按BOOT键，串口输出"按键按下!"和累计次数
- 不会因为抖动出现一次按下输出多次的情况

### 任务5.3: I2C总线 -- 读取BMI270传感器 (50分钟)

ESP-IDF v5.x使用新版I2C驱动API。SparkBot使用BMI270六轴传感器（加速度计+陀螺仪），通过I2C总线连接。

ESP-IDF v5.x uses the new I2C driver API. SparkBot uses the BMI270 6-axis sensor (accelerometer + gyroscope) connected via I2C bus.

```c
#include "driver/i2c_master.h"
#include "esp_log.h"

static const char *TAG = "BMI270";

// I2C引脚定义 (根据SparkBot原理图)
#define I2C_SCL_IO      GPIO_NUM_41
#define I2C_SDA_IO      GPIO_NUM_42
#define BMI270_ADDR     0x68        // 7位地址 (SDO接GND)

// BMI270关键寄存器
#define BMI270_CHIP_ID      0x00    // 芯片ID (应为0x24)
#define BMI270_ACC_DATA     0x0C    // 加速度数据起始地址(6字节)
#define BMI270_GYR_DATA     0x12    // 陀螺仪数据起始地址(6字节)
#define BMI270_CMD          0x7E    // 命令寄存器
#define BMI270_PWR_CTRL     0x7D    // 电源控制

static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t dev_handle;

// I2C初始化 (ESP-IDF v5.x新版API)
static esp_err_t i2c_init(void)
{
    // 1. 配置I2C总线
    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_SCL_IO,
        .sda_io_num = I2C_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,  // 启用内部上拉
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus_handle));

    // 2. 添加BMI270设备
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BMI270_ADDR,
        .scl_speed_hz = 400000,  // 400kHz快速模式
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

    // 3. 探测设备是否存在
    ESP_ERROR_CHECK(i2c_master_probe(bus_handle, BMI270_ADDR, 100));
    ESP_LOGI(TAG, "I2C初始化成功，检测到BMI270 @ 0x%02X", BMI270_ADDR);
    return ESP_OK;
}

// 写BMI270寄存器
static esp_err_t bmi270_write_reg(uint8_t reg, uint8_t data)
{
    uint8_t buf[2] = {reg, data};
    return i2c_master_transmit(dev_handle, buf, sizeof(buf), 100);
}

// 读BMI270寄存器
static esp_err_t bmi270_read_reg(uint8_t reg, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(dev_handle, &reg, 1, data, len, 100);
}

// BMI270初始化
static esp_err_t bmi270_init(void)
{
    uint8_t chip_id = 0;
    ESP_ERROR_CHECK(bmi270_read_reg(BMI270_CHIP_ID, &chip_id, 1));
    ESP_LOGI(TAG, "BMI270 芯片ID: 0x%02X (期望: 0x24)", chip_id);

    if (chip_id != 0x24) {
        ESP_LOGE(TAG, "芯片ID不匹配! 请检查焊接");
        return ESP_FAIL;
    }

    // 软复位
    bmi270_write_reg(BMI270_CMD, 0xB6);
    vTaskDelay(pdMS_TO_TICKS(50));

    // 上电并设置为性能模式
    bmi270_write_reg(BMI270_PWR_CTRL, 0x04);
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "BMI270初始化完成!");
    return ESP_OK;
}

// 读取加速度数据 (单位: g)
static void bmi270_read_accel(float *ax, float *ay, float *az)
{
    uint8_t data[6];
    bmi270_read_reg(BMI270_ACC_DATA, data, 6);

    // 16位有符号, 小端序 → 转换为g值 (±2g量程: LSB/16384)
    int16_t rx = (int16_t)((data[1] << 8) | data[0]);
    int16_t ry = (int16_t)((data[3] << 8) | data[2]);
    int16_t rz = (int16_t)((data[5] << 8) | data[4]);

    *ax = rx / 16384.0f;
    *ay = ry / 16384.0f;
    *az = rz / 16384.0f;
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== SparkBot 传感器读取演示 ===");
    i2c_init();
    bmi270_init();

    while (1) {
        float ax, ay, az;
        bmi270_read_accel(&ax, &ay, &az);
        ESP_LOGI(TAG, "加速度: X=%+.2fg  Y=%+.2fg  Z=%+.2fg", ax, ay, az);

        // 姿态判断
        if (az > 0.8f)       ESP_LOGI(TAG, "  >> 正面朝上");
        else if (az < -0.8f) ESP_LOGI(TAG, "  >> 正面朝下");
        else if (ax > 0.8f)  ESP_LOGI(TAG, "  >> 向右倾斜");
        else if (ax < -0.8f) ESP_LOGI(TAG, "  >> 向左倾斜");

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

**预期结果:**
- 串口显示检测到BMI270芯片ID (0x24)
- 每100ms输出三轴加速度数据
- 转动SparkBot时数值实时变化
- 能识别正面朝上/朝下/倾斜姿态

**常见问题:**
- **i2c_master_probe 失败**：检查SDA/SCL引脚焊接是否良好，上拉电阻是否正确
- **芯片ID读出0xFF**：I2C通信未建立，检查引脚连接和电源
- **数据始终为0**：传感器未完成上电初始化，检查PWR_CTRL配置

---

## 下午: 触摸按键与任务间通信 | Afternoon: Touch Sensor & Inter-Task Comms

### 任务5.4: 触摸按键编程 (30分钟)

ESP32-S3内置电容触摸传感器，SparkBot使用FPC排线上的触摸按键。手指接近/触摸焊盘时电容值变化，ADC读数下降。

The ESP32-S3 has built-in capacitive touch sensors. SparkBot uses a touch pad on the FPC cable. When a finger touches the pad, capacitance changes and the ADC reading drops.

```c
#include "driver/touch_pad.h"
#include "esp_log.h"

static const char *TAG = "TOUCH";

// GPIO4对应TOUCH_PAD_NUM4
#define TOUCH_PAD_CHANNEL   TOUCH_PAD_NUM4
#define TOUCH_THRESHOLD     800     // 触摸阈值（需实际校准）

static void touch_callback(void *arg)
{
    uint32_t pad_status = 0;
    touch_pad_get_status(&pad_status);
    
    if (pad_status & (1 << TOUCH_PAD_CHANNEL)) {
        ESP_LOGI(TAG, "触摸检测: 被触摸了!");
        // 清除中断状态
        touch_pad_clear_status();
    }
}

void touch_init(void)
{
    // 1. 初始化触摸外设
    touch_pad_init();

    // 2. 配置触摸通道
    touch_pad_config(TOUCH_PAD_CHANNEL, 0);  // threshold=0, 用滤波阈值

    // 3. 启用软件滤波 (防干扰)
    touch_pad_filter_start(10);  // 10ms滤波周期

    // 4. 校准阈值
    // 第一步: 读取无触摸时的基准值
    uint32_t base_value;
    touch_pad_read_benchmark(TOUCH_PAD_CHANNEL, &base_value);
    // 第二步: 设置阈值 = 基准值 * 0.6 (触摸时值会降到此以下)
    touch_pad_set_thresh(TOUCH_PAD_CHANNEL, base_value * 0.6);

    // 5. 注册中断回调（可选）
    touch_pad_isr_register(touch_callback, NULL, 0, NULL);
    touch_pad_intr_enable(TOUCH_PAD_CHANNEL);

    ESP_LOGI(TAG, "触摸按键初始化完成 (基准值: %lu)", base_value);
}

// 触摸阈值调试: 串口实时打印触摸值
void touch_debug_loop(void)
{
    while (1) {
        uint32_t touch_val = 0;
        touch_pad_read_raw_data(TOUCH_PAD_CHANNEL, &touch_val);
        
        // 触摸时电容值下降，低于阈值
        bool pressed = touch_val < 15000;  // 示例值，需根据实际校准
        ESP_LOGI(TAG, "触摸原始值: %lu %s", touch_val, pressed ? "<< 触摸!" : "");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
```

**触摸阈值校准流程:**
1. 不触摸时记录基准值（通常是几十万量级）
2. 触摸时观察读数下降量（通常下降30%-60%）
3. 将阈值设在基准值和触摸值之间

### 任务5.5: 综合实战 -- 传感器数据采集系统 (40分钟)

使用FreeRTOS任务+队列构建完整的传感器数据采集流水线。两个任务分工协作：一个采集数据，一个处理显示。

Build a complete sensor data pipeline using FreeRTOS tasks + queues. Two tasks divide the work: one collects data, one processes and displays it.

```c
#include "freertos/queue.h"

// 传感器数据消息
typedef struct {
    float accel_x, accel_y, accel_z;
    bool  touch_pressed;
} sensor_data_t;

static QueueHandle_t sensor_queue;

// 任务A: 传感器采集任务 (高频率)
void sensor_collect_task(void *pv)
{
    sensor_data_t data;
    while (1) {
        // 读加速度
        bmi270_read_accel(&data.accel_x, &data.accel_y, &data.accel_z);

        // 读触摸
        uint32_t touch_val;
        touch_pad_read_raw_data(TOUCH_PAD_CHANNEL, &touch_val);
        data.touch_pressed = (touch_val < 15000);

        // 发送到队列 (不等待，满了就丢弃旧数据)
        xQueueOverwrite(sensor_queue, &data);

        vTaskDelay(pdMS_TO_TICKS(20));  // 50Hz采集
    }
}

// 任务B: LED显示任务 (根据传感器数据变化LED)
void led_display_task(void *pv)
{
    sensor_data_t data;
    TickType_t last_touch_time = 0;

    while (1) {
        // 从队列接收数据 (阻塞等待，最多等100ms)
        if (xQueueReceive(sensor_queue, &data, pdMS_TO_TICKS(100)) == pdTRUE) {

            // LED亮度反映加速度大小
            float magnitude = sqrtf(data.accel_x * data.accel_x +
                                    data.accel_y * data.accel_y +
                                    data.accel_z * data.accel_z);
            if (magnitude > 1.5f) {
                ESP_LOGI("LED", "检测到摇晃! 幅度=%.2f", magnitude);
                gpio_set_level(LED_GPIO, 1);   // 摇晃 → LED亮
            } else {
                gpio_set_level(LED_GPIO, 0);   // 静止 → LED灭
            }

            // 触摸控制LED闪烁
            if (data.touch_pressed) {
                ESP_LOGI("LED", "触摸→LED以2Hz闪烁");
                for (int i = 0; i < 4; i++) {
                    gpio_set_level(LED_GPIO, 1);
                    vTaskDelay(pdMS_TO_TICKS(250));
                    gpio_set_level(LED_GPIO, 0);
                    vTaskDelay(pdMS_TO_TICKS(250));
                }
            }
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== SparkBot 传感器采集系统 ===");

    // 硬件初始化
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    i2c_init();
    bmi270_init();
    touch_init();

    // 创建队列 (深度=1, 只保留最新数据)
    sensor_queue = xQueueCreate(1, sizeof(sensor_data_t));

    // 创建任务
    xTaskCreate(sensor_collect_task, "sensor", 4096, NULL, 2, NULL);
    xTaskCreate(led_display_task,  "led",    4096, NULL, 1, NULL);

    ESP_LOGI(TAG, "系统就绪! 摇晃或触摸SparkBot试试看");
}
```

**架构图:**
```
传感器采集任务 (优先级2, 50Hz)       LED显示任务 (优先级1)
        │                                    │
        ├─ BMI270加速度 ─┐                  ┌─ 摇晃检测→LED亮
        ├─ 触摸按键   ─┼── Queue ──→ 接收 ──┤
        │              │  (深度1)           └─ 触摸→LED闪烁
        └─ 20ms周期 ───┘
```

---

## 今日作业 | Homework

### 必做题
1. 完成LED GPIO控制，支持按键翻转LED状态
2. 完成I2C初始化，成功读取BMI270芯片ID (0x24)
3. 串口持续打印三轴加速度数据，转动SparkBot数值跟随变化
4. 实现触摸按键检测并在串口输出触摸状态

### 挑战题
1. **LED呼吸灯**: 使用PWM (LEDC)实现LED呼吸效果（渐亮渐灭）
2. **摇晃检测**: 当加速度矢量模长超过1.5g时触发"摇晃事件"，LED快速闪烁3次
3. **触摸+LED联动**: 单次触摸→LED闪烁2次，双击(500ms内) → LED常亮2秒，长按(>1s) → LED呼吸

### 思考题
1. I2C总线为什么需要上拉电阻？去掉上拉电阻会怎样？（提示：I2C使用开漏输出）
2. 中断方式和轮询方式读取按键各有什么优缺点？嵌入式系统中什么时候必须用中断？
3. 触摸按键的电容检测原理是什么？为什么手指靠近会改变电容值？

---

## 明日预告 | Tomorrow's Preview

明天我们将驱动ST7789显示屏！你将学习SPI总线通信，使用LVGL图形库创建漂亮的GUI界面，让SparkBot显示表情、文字和动画。这是SparkBot"活起来"的关键一步！

Tomorrow we will drive the ST7789 display! You will learn SPI bus communication, use the LVGL graphics library to create GUI interfaces, and make SparkBot show expressions, text, and animations. This is the key step to making SparkBot "come alive"!

---

## 参考资源 | References

- [ESP-IDF GPIO 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/peripherals/gpio.html)
- [ESP-IDF I2C 新版驱动指南](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/peripherals/i2c_master.html)
- [ESP-IDF 触摸传感器指南](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/peripherals/touch_pad.html)
- [FreeRTOS 队列 API 文档](https://www.freertos.org/Embedded-RTOS-Queues.html)
- [BMI270 数据手册](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmi270-ds000.pdf)
- [ESP32 I2C 通信详解 (B站)](https://search.bilibili.com/all?keyword=ESP32%20I2C%20%E9%80%9A%E4%BF%A1)

*最后更新：2026-05-27*
