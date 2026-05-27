# Day 07: 摄像头与图像处理 | Camera & Image Processing

> **今日目标:**
> - 掌握DVP接口摄像头驱动
> - 拍照并保存JPEG图片
> - ESP-WHO人脸检测入门
> - 移动检测实现
>
> **产出:** OV2640摄像头成功拍照，在LCD上显示预览画面，人脸检测框出人脸，实现移动检测触发告警

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间活动 | 显示界面展示与传感器联动演示 |
| 09:15 - 10:30 | 知识讲解 | DVP接口原理、OV2640传感器、图像格式(JPEG/RGB/YUV) |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | 摄像头初始化、拍照、在LCD上显示照片 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | ESP-WHO人脸检测集成、实时人脸框绘制 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 移动检测(帧差法)、安防监控雏形 |
| 16:30 - 17:00 | 总结分享 | 摄像头效果展示、人脸检测精度讨论 |

---

## 上午: OV2640摄像头驱动 | Morning: OV2640 Camera Driver

### 为什么要学这个? | Why Learn This?

摄像头让SparkBot真正拥有"视觉"能力。它能拍照记录、检测人脸、感知环境变化。DVP (Digital Video Port) 是ESP32-S3的并行摄像头接口，支持8位数据总线，能以每秒数十帧的速度接收图像数据。

The camera gives SparkBot real "vision". It can take photos, detect faces, and sense environmental changes. DVP is ESP32-S3's parallel camera interface with an 8-bit data bus, receiving image data at tens of frames per second.

**DVP接口架构:**
```
OV2640摄像头                      ESP32-S3 DVP外设
    PCLK ───────────────────────> 像素时钟
    HSYNC ──────────────────────> 行同步(HREF)
    VSYNC ──────────────────────> 帧同步
    D0-D7 ─────────────────────> 8位并行数据总线
    XCLK <─────────────────────── 主时钟(20MHz)
    SIOD ─────────────────────── SCCB SDA (类I2C配置接口)
    SIOC ─────────────────────── SCCB SCL
```

### 任务7.1: 摄像头初始化与拍照 (50分钟)

```c
#include "esp_camera.h"
#include "esp_log.h"
#include "esp_spiffs.h"

static const char *TAG = "CAMERA";

// OV2640 引脚定义 (根据SparkBot原理图)
#define CAM_PIN_PWDN    -1          // 未使用
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK    43
#define CAM_PIN_SIOD    44          // SCCB SDA
#define CAM_PIN_SIOC    45          // SCCB SCL

#define CAM_PIN_D0      39          // DVP 8位数据线
#define CAM_PIN_D1      40
#define CAM_PIN_D2      41
#define CAM_PIN_D3      42
#define CAM_PIN_D4      35
#define CAM_PIN_D5      36
#define CAM_PIN_D6      37
#define CAM_PIN_D7      38

#define CAM_PIN_VSYNC   21
#define CAM_PIN_HREF    47
#define CAM_PIN_PCLK    48

static camera_config_t camera_config = {
    .pin_pwdn  = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk  = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7, .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5, .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3, .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1, .pin_d0 = CAM_PIN_D0,

    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href  = CAM_PIN_HREF,
    .pin_pclk  = CAM_PIN_PCLK,

    .xclk_freq_hz = 20000000,       // 20MHz XCLK
    .ledc_timer   = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, // JPEG: 省内存和存储
    .frame_size   = FRAMESIZE_QVGA, // 320x240

    .jpeg_quality = 12,             // 0-63 (越小质量越好)
    .fb_count     = 2,              // 双帧缓冲
    .fb_location  = CAMERA_FB_IN_PSRAM,
    .grab_mode    = CAMERA_GRAB_WHEN_EMPTY,
};

// 摄像头初始化
static esp_err_t camera_init(void)
{
    ESP_LOGI(TAG, "初始化OV2640摄像头...");

    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "摄像头初始化失败! err=0x%x", err);
        return err;
    }

    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        ESP_LOGI(TAG, "传感器PID: 0x%02x (OV2640=0x26)", s->id.PID);
    }
    ESP_LOGI(TAG, "OV2640初始化成功!");
    return ESP_OK;
}

// 拍照获取一帧
static camera_fb_t *camera_take_photo(void)
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "获取摄像头帧失败!");
        return NULL;
    }
    ESP_LOGI(TAG, "拍照成功! %dx%d, %zu bytes, fmt=%d",
             fb->width, fb->height, fb->len, fb->format);
    return fb;
}

// 保存JPEG到SPIFFS
static esp_err_t save_jpeg_to_spiffs(const char *filename, camera_fb_t *fb)
{
    if (fb->format != PIXFORMAT_JPEG) {
        ESP_LOGE(TAG, "非JPEG格式,无法直接保存");
        return ESP_ERR_INVALID_ARG;
    }

    FILE *f = fopen(filename, "wb");
    if (!f) { ESP_LOGE(TAG, "无法创建文件: %s", filename); return ESP_FAIL; }

    fwrite(fb->buf, 1, fb->len, f);
    fclose(f);
    ESP_LOGI(TAG, "照片已保存: %s (%zu bytes)", filename, fb->len);
    return ESP_OK;
}

// 显示JPEG在LCD上 (LVGL)
static void display_jpeg_on_lcd(camera_fb_t *fb)
{
    if (fb->format != PIXFORMAT_JPEG) return;

    lv_obj_t *img = lv_img_create(lv_scr_act());
    lv_img_dsc_t img_dsc = {
        .header.cf = LV_IMG_CF_RAW,
        .header.w = fb->width,
        .header.h = fb->height,
        .data_size = fb->len,
        .data = fb->buf,
    };
    lv_img_set_src(img, &img_dsc);
    lv_obj_center(img);
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== SparkBot 摄像头演示 ===");

    // 挂载SPIFFS
    esp_vfs_spiffs_conf_t spiffs_cfg = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true,
    };
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&spiffs_cfg));

    // 初始化摄像头
    ESP_ERROR_CHECK(camera_init());

    // 拍照 → 保存 → 显示
    camera_fb_t *fb = camera_take_photo();
    if (fb) {
        save_jpeg_to_spiffs("/spiffs/photo.jpg", fb);
        display_jpeg_on_lcd(fb);
        esp_camera_fb_return(fb);  // 归还帧缓冲 (必须!)
    }
}
```

**图像格式对比:**

| 格式 | 大小(320x240) | 适合场景 |
|------|--------------|---------|
| RGB565 | 150KB | 直接显示、图像处理 |
| JPEG | 5-20KB | 拍照存储、网络传输 |
| YUV422 | 150KB | 人脸检测、机器视觉 |

**预期结果:**
- 串口显示传感器PID=0x26 (OV2640)
- 拍照成功并保存JPEG文件到SPIFFS
- 照片显示在LCD屏幕上

**常见问题:**
- **"Camera init failed"**：最常见是DVP数据线有虚焊。用万用表逐根检查D0-D7通断
- **SCCB通信失败**：检查SIOD/SIOC引脚，OV2640 I2C地址为0x30
- **图像花屏**：DVP数据线顺序错误或FPC排线接触不良
- **PSRAM不足**：确认menuconfig启用 `CONFIG_SPIRAM=y` 及8MB PSRAM

### 任务7.2: 摄像头参数调节 (30分钟)

```c
static void camera_adjust_params(void)
{
    sensor_t *s = esp_camera_sensor_get();
    if (!s) return;

    // ---- 自动控制 ----
    s->set_wb_mode(s, 0);          // 0=自动白平衡
    s->set_exposure_ctrl(s, 1);    // 1=自动曝光
    s->set_gain_ctrl(s, 1);        // 1=自动增益
    s->set_aec2(s, 1);             // 启用夜间自动曝光

    // ---- 画质参数 ----
    s->set_brightness(s, 0);       // 亮度: -2~2
    s->set_contrast(s, 0);         // 对比度: -2~2
    s->set_saturation(s, 0);       // 饱和度: -2~2
    s->set_quality(s, 12);         // JPEG质量

    // ---- 特效 ----
    s->set_special_effect(s, 0);   // 0=无 2=黑白 3=负片 4=红调

    // ---- 分辨率切换 ----
    // UXGA(1600x1200) — 高分辨率拍照
    s->set_framesize(s, FRAMESIZE_UXGA);
    vTaskDelay(pdMS_TO_TICKS(300));
    camera_fb_t *fb = esp_camera_fb_get();
    if (fb) { esp_camera_fb_return(fb); }

    // QVGA(320x240) — 低分辨率实时预览
    s->set_framesize(s, FRAMESIZE_QVGA);

    // ---- 镜像/翻转 (根据摄像头物理安装方向调整) ----
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);

    ESP_LOGI(TAG, "摄像头参数配置完成!");
}

// 帧率统计
static void preview_fps_counter(void)
{
    static int frame_count = 0;
    static uint32_t last_time = 0;
    frame_count++;

    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    if (now - last_time >= 5000) {
        float fps = frame_count * 1000.0f / (now - last_time);
        ESP_LOGI(TAG, "预览帧率: %.1f fps @ QVGA", fps);
        frame_count = 0;
        last_time = now;
    }
}
```

---

## 下午: AI视觉入门 | Afternoon: AI Vision Basics

### 任务7.3: 人脸检测 — ESP-WHO (40分钟)

ESP-WHO是乐鑫官方的人脸检测/识别框架，使用轻量级MTMN模型，针对ESP32-S3向量指令集优化。

ESP-WHO is Espressif's official face detection/recognition framework using the lightweight MTMN model, optimized for ESP32-S3 vector instructions.

```c
// 添加依赖: idf_component.yml
// dependencies:
//   espressif/esp-who: "^1.0.0"

#include "esp_who.h"

// 人脸检测结果消息
typedef struct {
    int x, y, w, h;
    float score;
} face_box_t;

static QueueHandle_t face_queue;

// 人脸检测任务
static void face_detect_task(void *pv)
{
    // 注: ESP-WHO详细API请参考官方示例
    // esp-who/examples/human_face_detection/lcd/
    //
    // 核心流程:
    // 1. 创建MTMN检测器: TwoStageDetector *detector = new TwoStageDetector(...)
    // 2. 获取摄像头RGB帧
    // 3. 调用 detector->detect(image, boxes)
    // 4. 遍历boxes，提取每张人脸的坐标和置信度
    // 5. 将结果通过队列发送给LCD绘制任务

    while (1) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) {
            // ... 人脸检测处理 ...
            esp_camera_fb_return(fb);
        }
        vTaskDelay(pdMS_TO_TICKS(100));  // ~10fps
    }
}

// 在LCD上绘制人脸框 (LVGL)
static void draw_face_box(int x, int y, int w, int h)
{
    lv_obj_t *box = lv_obj_create(lv_scr_act());
    lv_obj_set_size(box, w, h);
    lv_obj_set_pos(box, x, y);
    lv_obj_set_style_border_color(box, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_border_width(box, 3, 0);
    lv_obj_set_style_bg_opa(box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(box, 0, 0);

    // 3秒后自动移除框
    lv_obj_t *timer_label = lv_label_create(box);
    lv_label_set_text_fmt(timer_label, "Face %dx%d", w, h);
    lv_obj_set_style_text_color(timer_label, lv_color_hex(0x00FF00), 0);
}

// 完整示例参考:
// https://github.com/espressif/esp-who/tree/master/examples/human_face_detection/lcd
```

**ESP-WHO安装步骤:**
1. 在项目根目录执行: `idf.py add-dependency "espressif/esp-who^1.0.0"`
2. 将示例代码中的 `app_main.cpp` 改为C++文件或参考其结构在C中调用
3. 人脸检测模型文件会自动下载到 `managed_components/` 目录

### 任务7.4: 移动检测 — 帧差法 (30分钟)

帧差法是计算机视觉中最简单高效的移动检测方法：比较连续两帧的像素差异。

Frame differencing: compare pixel differences between consecutive frames. Simple and efficient for motion detection.

```c
#define MOTION_THRESHOLD    30      // 像素差异阈值
#define MOTION_RATIO        0.10f   // 10%像素变化 → 移动

static uint8_t *prev_frame = NULL;

static void motion_detect_init(void)
{
    prev_frame = calloc(1, 320 * 240);  // QVGA灰度图
    ESP_LOGI(TAG, "移动检测初始化 (帧差法, 阈值=%d)", MOTION_THRESHOLD);
}

// RGB565转灰度
static void rgb565_to_gray(const uint8_t *rgb565, uint8_t *gray, int pixels)
{
    for (int i = 0; i < pixels; i++) {
        uint16_t p = ((uint16_t *)rgb565)[i];
        uint8_t r = (p >> 11) & 0x1F;
        uint8_t g = (p >> 5)  & 0x3F;
        uint8_t b = p & 0x1F;
        gray[i] = (r * 77 + g * 150 + b * 29) >> 8;  // 加权灰度
    }
}

// 帧差法检测
static bool detect_motion(camera_fb_t *fb)
{
    if (!fb || fb->format != PIXFORMAT_RGB565 || !prev_frame) return false;

    int total = fb->width * fb->height;
    uint8_t *current_gray = malloc(total);
    rgb565_to_gray(fb->buf, current_gray, total);

    // 逐像素比较
    int diff_pixels = 0;
    for (int i = 0; i < total; i++) {
        if (abs((int)current_gray[i] - (int)prev_frame[i]) > MOTION_THRESHOLD) {
            diff_pixels++;
        }
    }

    memcpy(prev_frame, current_gray, total);
    free(current_gray);

    float ratio = (float)diff_pixels / total;
    bool motion = ratio > MOTION_RATIO;

    if (motion) {
        ESP_LOGI(TAG, "检测到移动! 变化: %d/%d pixels (%.1f%%)",
                 diff_pixels, total, ratio * 100);
    }
    return motion;
}

// 安防监控任务
static void motion_monitor_task(void *pv)
{
    motion_detect_init();
    TickType_t last_alert = 0;

    while (1) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) { vTaskDelay(pdMS_TO_TICKS(100)); continue; }

        if (detect_motion(fb)) {
            TickType_t now = xTaskGetTickCount();
            // 防抖: 5秒内不重复告警
            if (now - last_alert > pdMS_TO_TICKS(5000)) {
                last_alert = now;

                // 触发LED闪烁告警
                gpio_set_level(GPIO_NUM_48, 1);
                vTaskDelay(pdMS_TO_TICKS(200));
                gpio_set_level(GPIO_NUM_48, 0);

                ESP_LOGW(TAG, "安防告警: 检测到移动!");
            }
        }

        esp_camera_fb_return(fb);
        vTaskDelay(pdMS_TO_TICKS(200));  // 5fps
    }
}
```

---

## 今日作业 | Homework

### 必做题
1. 完成OV2640摄像头初始化，读取并验证传感器PID (0x26)
2. 拍照并保存JPEG到SPIFFS，显示在LCD上
3. 实现帧差法移动检测基础代码，能在串口输出变化像素比例

### 挑战题
1. **自拍功能**: 用BOOT按键触发拍照，以时间戳命名文件 (如 `20260527_143025.jpg`)，LCD短暂闪烁作为快门效果
2. **人脸计数**: 集成ESP-WHO人脸检测，在串口实时打印画面中人脸数量和第一张人脸位置
3. **移动检测+声光告警**: 检测到移动时LED闪烁3次，LCD显示"Motion detected!"告警横幅

### 思考题
1. DVP和MIPI CSI接口有什么区别？为什么ESP32-S3选择DVP而非MIPI？
2. 人脸检测和"人脸识别"有什么本质区别？ESP-WHO能做哪个、不能做哪个？
3. 帧差法检测移动有哪些局限性？什么情况下会误报或漏报？

---

## 明日预告 | Tomorrow's Preview

明天我们进入音频世界！学习I2S数字音频总线、ES8311编解码器编程，让SparkBot具备录音和播放能力。重头戏是——将SparkBot变成一个蓝牙A2DP音箱，通过手机播放音乐！

Tomorrow we enter the audio world! Learn I2S digital audio bus and ES8311 codec programming to give SparkBot recording and playback. The highlight -- turning SparkBot into a Bluetooth A2DP speaker to play music from your phone!

---

## 参考资源 | References

- [ESP32-Camera 驱动文档](https://docs.espressif.com/projects/esp32-camera/en/latest/)
- [ESP-WHO 人脸检测框架 (GitHub)](https://github.com/espressif/esp-who)
- [OV2640 数据手册](https://www.uctronics.com/download/cam_module/OV2640DS.pdf)
- [ESP32-S3 DVP 接口编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/peripherals/dvp.html)
- [ESP-IDF SPIFFS 文件系统指南](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/storage/spiffs.html)
- [ESP32-CAM 入门教程 (B站)](https://search.bilibili.com/all?keyword=ESP32-CAM%20%E6%94%9D%E5%83%8F%E9%A0%AD)
- [帧差法原理与实现 (B站)](https://search.bilibili.com/all?keyword=%E5%B8%A7%E5%B7%AE%E6%B3%95%20%E7%A7%BB%E5%8A%A8%E6%A3%80%E6%B5%8B)

*最后更新：2026-05-27*
