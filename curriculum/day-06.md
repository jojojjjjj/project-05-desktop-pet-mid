# Day 06: 显示屏驱动与GUI开发 | Display Driver & GUI Development

> **今日目标:**
> - 掌握SPI总线协议
> - 驱动ST7789显示屏 (240x240)
> - LVGL图形库入门
> - 实现天气时钟界面和表情动画
>
> **产出:** ST7789成功点亮并显示彩色图像，LVGL界面运行，实现天气时钟和宠物表情

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间活动 | 传感器数据采集系统演示、显示效果预期 |
| 09:15 - 10:30 | 知识讲解 | SPI总线协议、ST7789驱动原理、LVGL架构 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | ST7789初始化、纯色填充、图片显示 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | LVGL移植与控件使用、事件回调 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 天气时钟界面设计、表情动画切换 |
| 16:30 - 17:00 | 总结分享 | 各组GUI界面展示、代码评审 |

---

## 上午: SPI总线与ST7789驱动 | Morning: SPI Bus & ST7789 Driver

### 为什么要学这个? | Why Learn This?

屏幕是SparkBot与用户交互的最主要界面。所有信息——天气、时间、AI回复、表情——都通过屏幕展示。SPI (Serial Peripheral Interface) 是嵌入式系统中最常见的高速通信协议，比I2C更快，适合显示屏这类大数据量外设。LVGL则是嵌入式GUI的事实标准，被NXP、小米、华为等公司广泛使用。

The display is SparkBot's main user interface. All information -- weather, time, AI replies, expressions -- goes through the screen. SPI is the most common high-speed protocol for data-intensive peripherals. LVGL is the de facto standard for embedded GUI, used by NXP, Xiaomi, Huawei, and more.

### 任务6.1: SPI总线与ST7789初始化 (50分钟)

ST7789是一个240x240像素的TFT-LCD驱动芯片，通过SPI接口与ESP32-S3通信。

```c
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "ST7789";

// SPI引脚定义 (根据SparkBot原理图)
#define PIN_MOSI        GPIO_NUM_11
#define PIN_SCLK        GPIO_NUM_12
#define PIN_CS          GPIO_NUM_10    // 片选
#define PIN_DC          GPIO_NUM_14    // 数据/命令选择
#define PIN_RST         GPIO_NUM_9     // 复位
#define PIN_BL          GPIO_NUM_8     // 背光

#define LCD_WIDTH       240
#define LCD_HEIGHT      240

static spi_device_handle_t spi;

// 写命令 (DC引脚拉低)
static void lcd_write_cmd(uint8_t cmd)
{
    gpio_set_level(PIN_DC, 0);
    spi_transaction_t t = { .length = 8, .tx_buffer = &cmd };
    spi_device_polling_transmit(spi, &t);
}

// 写数据 (DC引脚拉高)
static void lcd_write_data(const uint8_t *data, size_t len)
{
    gpio_set_level(PIN_DC, 1);
    spi_transaction_t t = { .length = len * 8, .tx_buffer = data };
    spi_device_polling_transmit(spi, &t);
}

// ST7789初始化序列
static void st7789_init(void)
{
    // 硬件复位
    gpio_set_level(PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    // 软件复位
    lcd_write_cmd(0x01);  // SWRESET
    vTaskDelay(pdMS_TO_TICKS(150));

    // 退出休眠模式
    lcd_write_cmd(0x11);  // SLPOUT
    vTaskDelay(pdMS_TO_TICKS(120));

    // 像素格式: 16位RGB565
    lcd_write_cmd(0x3A);  // COLMOD
    uint8_t pfmt = 0x55; lcd_write_data(&pfmt, 1);

    // 显示反转开启 (ST7789通常需要)
    lcd_write_cmd(0x21);  // INVON

    // 普通显示模式
    lcd_write_cmd(0x13);  // NORON

    // 开启显示
    lcd_write_cmd(0x29);  // DISPON
    vTaskDelay(pdMS_TO_TICKS(50));

    // 打开背光
    gpio_set_level(PIN_BL, 1);

    ESP_LOGI(TAG, "ST7789 初始化完成!");
}

// 设置绘图窗口区域
static void lcd_set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint16_t xe = x + w - 1, ye = y + h - 1;
    uint8_t data[4];

    lcd_write_cmd(0x2A);  // CASET: 列地址
    data[0] = x >> 8;  data[1] = x & 0xFF;
    data[2] = xe >> 8; data[3] = xe & 0xFF;
    lcd_write_data(data, 4);

    lcd_write_cmd(0x2B);  // RASET: 行地址
    data[0] = y >> 8;  data[1] = y & 0xFF;
    data[2] = ye >> 8; data[3] = ye & 0xFF;
    lcd_write_data(data, 4);

    lcd_write_cmd(0x2C);  // RAMWR: 写内存
}

// 填充纯色
static void lcd_fill_color(uint16_t color)
{
    lcd_set_window(0, 0, LCD_WIDTH, LCD_HEIGHT);

    uint16_t *line = heap_caps_malloc(LCD_WIDTH * 2, MALLOC_CAP_DMA);
    for (int i = 0; i < LCD_WIDTH; i++) line[i] = color;

    gpio_set_level(PIN_DC, 1);
    for (int y = 0; y < LCD_HEIGHT; y++) {
        spi_transaction_t t = { .length = LCD_WIDTH * 16, .tx_buffer = line };
        spi_device_polling_transmit(spi, &t);
    }
    free(line);
}

void app_main(void)
{
    // GPIO初始化
    gpio_reset_pin(PIN_DC);  gpio_set_direction(PIN_DC, GPIO_MODE_OUTPUT);
    gpio_reset_pin(PIN_RST); gpio_set_direction(PIN_RST, GPIO_MODE_OUTPUT);
    gpio_reset_pin(PIN_BL);  gpio_set_direction(PIN_BL, GPIO_MODE_OUTPUT);

    // SPI总线初始化
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = -1,        // ST7789不需要MISO
        .sclk_io_num = PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * 2 + 8,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    // 添加ST7789设备
    spi_device_interface_config_t dev_cfg = {
        .mode = 3,                          // SPI Mode 3 (CPOL=1, CPHA=1)
        .clock_speed_hz = 40 * 1000 * 1000, // 40MHz
        .spics_io_num = PIN_CS,
        .queue_size = 7,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI3_HOST, &dev_cfg, &spi));

    // 初始化和测试
    st7789_init();

    ESP_LOGI(TAG, "颜色测试: 红→绿→蓝→白→黑");
    lcd_fill_color(0xF800);  vTaskDelay(pdMS_TO_TICKS(500));  // 红色
    lcd_fill_color(0x07E0);  vTaskDelay(pdMS_TO_TICKS(500));  // 绿色
    lcd_fill_color(0x001F);  vTaskDelay(pdMS_TO_TICKS(500));  // 蓝色
    lcd_fill_color(0xFFFF);  vTaskDelay(pdMS_TO_TICKS(500));  // 白色
    lcd_fill_color(0x0000);                                     // 黑色
    ESP_LOGI(TAG, "ST7789测试完成!");
}
```

**RGB565颜色编码:**
```
16位颜色: RRRRR GGGGGG BBBBB
0xF800 = 红, 0x07E0 = 绿, 0x001F = 蓝
0xFFFF = 白, 0x0000 = 黑
```

**SPI四种模式对比:**

| 模式 | CPOL | CPHA | 数据采样时机 | 常见设备 |
|------|------|------|-------------|----------|
| 0 | 0 | 0 | 上升沿 | 大部分SPI设备 |
| 1 | 0 | 1 | 下降沿 | 少数ADC |
| 2 | 1 | 0 | 下降沿 | 少数Flash |
| 3 | 1 | 1 | 上升沿 | **ST7789** |

**预期结果:**
- 屏幕背光亮起，依次显示红、绿、蓝、白、黑五种纯色
- 无花屏、条纹或颜色异常

**常见问题:**
- **白屏无图像**：检查RST复位时序，延时至少120ms
- **花屏**：检查SPI模式是否正确（ST7789用Mode 3: CPOL=1, CPHA=1）
- **颜色不对**：检查RGB565字节序，可能需要交换高低字节

### 任务6.2: 显示图片和文字 (30分钟)

```c
// 绘制像素点
static void lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    lcd_set_window(x, y, 1, 1);
    lcd_write_data((uint8_t *)&color, 2);
}

// 绘制填充矩形
static void lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    lcd_set_window(x, y, w, h);
    uint16_t *line = heap_caps_malloc(w * 2, MALLOC_CAP_DMA);
    for (int i = 0; i < w; i++) line[i] = color;
    gpio_set_level(PIN_DC, 1);
    for (int row = 0; row < h; row++) {
        spi_transaction_t t = { .length = w * 16, .tx_buffer = line };
        spi_device_polling_transmit(spi, &t);
    }
    free(line);
}

// 绘制像素级表情 — 开心脸
static void draw_emoji_happy(uint16_t x, uint16_t y, uint8_t scale)
{
    uint16_t yellow = 0xFFE0, black = 0x0000;
    lcd_fill_rect(x, y, 16 * scale, 16 * scale, yellow);
    // 左眼
    lcd_fill_rect(x + 3 * scale, y + 4 * scale, 2 * scale, 2 * scale, black);
    // 右眼
    lcd_fill_rect(x + 11 * scale, y + 4 * scale, 2 * scale, 2 * scale, black);
    // 微笑弧线
    for (int i = 0; i < 6; i++) {
        lcd_draw_pixel(x + (5 + i) * scale, y + 12 * scale, black);
    }
}

// 绘制像素级表情 — 难过脸
static void draw_emoji_sad(uint16_t x, uint16_t y, uint8_t scale)
{
    uint16_t yellow = 0xFFE0, black = 0x0000;
    lcd_fill_rect(x, y, 16 * scale, 16 * scale, yellow);
    lcd_fill_rect(x + 3 * scale, y + 4 * scale, 2 * scale, 2 * scale, black);
    lcd_fill_rect(x + 11 * scale, y + 4 * scale, 2 * scale, 2 * scale, black);
    for (int i = 0; i < 6; i++) {
        lcd_draw_pixel(x + (5 + i) * scale, y + 11 * scale, black);
    }
}

// 在LCD上画圆形 (Bresenham中点圆算法)
static void lcd_draw_circle(uint16_t cx, uint16_t cy, uint16_t r, uint16_t color)
{
    int x = 0, y = r, d = 3 - 2 * r;
    while (y >= x) {
        lcd_draw_pixel(cx + x, cy + y, color); lcd_draw_pixel(cx - x, cy + y, color);
        lcd_draw_pixel(cx + x, cy - y, color); lcd_draw_pixel(cx - x, cy - y, color);
        lcd_draw_pixel(cx + y, cy + x, color); lcd_draw_pixel(cx - y, cy + x, color);
        lcd_draw_pixel(cx + y, cy - x, color); lcd_draw_pixel(cx - y, cy - x, color);
        x++;
        if (d > 0) { y--; d += 4 * (x - y) + 10; }
        else { d += 4 * x + 6; }
    }
}
```

---

## 下午: LVGL图形库入门 | Afternoon: LVGL Graphics Library

### 任务6.3: LVGL移植与基础控件 (50分钟)

LVGL (Light and Versatile Graphics Library) 移植需要实现三个关键接口：像素刷新回调、输入读取、时钟基准。

```c
#include "lvgl.h"

// LVGL显示缓冲 (1/10屏幕行 = 双缓冲)
#define LVGL_BUF_SIZE  (LCD_WIDTH * LCD_HEIGHT / 10)
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[LVGL_BUF_SIZE];
static lv_color_t buf2[LVGL_BUF_SIZE];

// 像素刷新回调 — 连接LVGL和ST7789硬件
static void lvgl_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area,
                          lv_color_t *color_p)
{
    uint16_t w = area->x2 - area->x1 + 1;
    uint16_t h = area->y2 - area->y1 + 1;
    lcd_set_window(area->x1, area->y1, w, h);
    lcd_write_data((uint8_t *)color_p, w * h * 2);
    lv_disp_flush_ready(disp);  // 通知LVGL: 刷新完成
}

// 时钟基准 (每1ms调用一次)
static void lvgl_tick_cb(void *arg)
{
    lv_tick_inc(1);
}

void lvgl_init(void)
{
    lv_init();

    // 初始化显示缓冲
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LVGL_BUF_SIZE);

    // 注册显示驱动
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // 注册1ms定时器
    const esp_timer_create_args_t timer_args = {
        .callback = lvgl_tick_cb, .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_timer;
    esp_timer_create(&timer_args, &lvgl_timer);
    esp_timer_start_periodic(lvgl_timer, 1000);

    ESP_LOGI(TAG, "LVGL 初始化完成!");
}

// 创建基础演示界面
void create_demo_ui(void)
{
    // 深色背景
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x1a1a2e), 0);

    // 标题标签
    lv_obj_t *title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "ESP-SparkBot");
    lv_obj_set_style_text_color(title, lv_color_hex(0x00FF88), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // 创建按钮
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 120, 50);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x4CAF50), 0);

    // 按钮上的标签
    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Hello!");
    lv_obj_center(btn_label);

    // 按钮点击事件回调
    lv_obj_add_event_cb(btn, [](lv_event_t *e) {
        ESP_LOGI("UI", "按钮被点击!");
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_label_set_text(label, "Clicked!");
    }, LV_EVENT_CLICKED, NULL);

    // 图片控件 (显示LVGL内置符号)
    lv_obj_t *icon = lv_label_create(lv_scr_act());
    lv_label_set_text(icon, LV_SYMBOL_HOME);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFD700), 0);
    lv_obj_align(icon, LV_ALIGN_BOTTOM_MID, 0, -20);
}

// LVGL主循环任务
void lvgl_task(void *pv)
{
    while (1) {
        lv_timer_handler();  // 驱动LVGL所有定时器和渲染
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
```

### 任务6.4: 天气时钟界面 (40分钟)

```c
static lv_obj_t *time_label, *date_label, *temp_label, *weather_icon;

void create_weather_clock(void)
{
    // 深色主题背景
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x0f3460), 0);

    // 天气图标
    weather_icon = lv_label_create(lv_scr_act());
    lv_label_set_text(weather_icon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_font(weather_icon, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(weather_icon, lv_color_hex(0xFFD700), 0);
    lv_obj_align(weather_icon, LV_ALIGN_TOP_MID, 0, 10);

    // 时间 — 大字体
    time_label = lv_label_create(lv_scr_act());
    lv_label_set_text(time_label, "12:00");
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -10);

    // 日期
    date_label = lv_label_create(lv_scr_act());
    lv_label_set_text(date_label, "2026-05-27 周三");
    lv_obj_set_style_text_color(date_label, lv_color_hex(0xAAAAAA), 0);
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 25);

    // 温度
    temp_label = lv_label_create(lv_scr_act());
    lv_label_set_text(temp_label, "25 C");
    lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(temp_label, lv_color_hex(0x4FC3F7), 0);
    lv_obj_align(temp_label, LV_ALIGN_BOTTOM_MID, 0, -20);
}

// 定时刷新 — 每秒调用
static void clock_update_cb(lv_timer_t *timer)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char buf[32];

    // 时:分
    strftime(buf, sizeof(buf), "%H:%M", &timeinfo);
    lv_label_set_text(time_label, buf);

    // 日期 + 中文星期
    const char *week[] = {"周日","周一","周二","周三","周四","周五","周六"};
    snprintf(buf, sizeof(buf), "%d-%02d-%02d %s",
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1,
             timeinfo.tm_mday, week[timeinfo.tm_wday]);
    lv_label_set_text(date_label, buf);

    // 温度 (模拟波动 — 实际应从传感器/API获取)
    static float temp = 25.0f;
    temp += (rand() % 100 - 50) / 100.0f;
    snprintf(buf, sizeof(buf), "%.1f C", temp);
    lv_label_set_text(temp_label, buf);
}

void app_main(void)
{
    // ... SPI, ST7789, LVGL初始化 ...
    create_weather_clock();
    lv_timer_create(clock_update_cb, 1000, NULL);  // 每1秒
    xTaskCreate(lvgl_task, "lvgl", 8192, NULL, 1, NULL);
}
```

**预期结果:**
- 深色主题的天气时钟界面
- 时间每秒刷新，日期含中文星期
- 温度动态显示在底部
- 界面布局居中协调

**常见问题:**
- **LVGL编译失败**：确认 `idf_component.yml` 有 `lvgl/lvgl: "^8.3"` 依赖
- **中文不显示**：LVGL默认字体不含中文，需加载中文字库
- **屏幕刷新慢**：增大缓冲区到1/4屏幕或使用`spi_device_queue_transmit`异步传输

---

## 今日作业 | Homework

### 必做题
1. 完成ST7789初始化，屏幕依次显示红绿蓝白黑五色
2. 完成LVGL移植，显示"Hello SparkBot"和一个可点击按钮
3. 实现天气时钟界面，时间每秒更新

### 挑战题
1. **宠物表情界面**: 用LVGL控件设计至少4种宠物表情（开心、难过、惊讶、生气），包含不同的图标、颜色和文字
2. **表情切换动画**: 使用LVGL动画API (`lv_anim_t`) 实现表情淡入淡出切换，包括缩放和透明度变化
3. **传感器联动**: 将Day 5的BMI270加速度数据接入，摇晃SparkBot时屏幕自动切换到"头晕"表情

### 思考题
1. 为什么显示屏用SPI而不是I2C？SPI和I2C在速度、引脚数、架构上有什么本质区别？
2. LVGL的 `lv_timer_handler()` 为什么需要在任务循环中反复调用？内部在做什么工作？
3. LVGL的双缓冲机制如何减少画面撕裂？单缓冲 vs 双缓冲的权衡是什么？

---

## 明日预告 | Tomorrow's Preview

明天我们将为SparkBot装上"眼睛"！通过OV2640摄像头，SparkBot将获得拍照和实时视频能力。你还会学习ESP-WHO人脸检测框架，让SparkBot能识别并框出画面中的人脸！

Tomorrow we give SparkBot "eyes"! With the OV2640 camera, SparkBot gains photo and live video capabilities. You will also learn the ESP-WHO face detection framework, enabling SparkBot to detect and mark faces in the frame!

---

## 参考资源 | References

- [ESP-IDF SPI 驱动指南](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/peripherals/spi_master.html)
- [ST7789V 数据手册](https://newhavendisplay.com/content/datasheets/ST7789V.pdf)
- [LVGL 官方文档](https://docs.lvgl.io/master/index.html)
- [LVGL 中文教程 (百问网)](https://lvgl.100ask.net/)
- [LVGL 图片在线转换工具](https://lvgl.io/tools/imageconverter)
- [ESP32 LVGL 实战教程 (B站)](https://search.bilibili.com/all?keyword=ESP32%20LVGL%20%E6%95%99%E7%A8%8B)

*最后更新：2026-05-27*
