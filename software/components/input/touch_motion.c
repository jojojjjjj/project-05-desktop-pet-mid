/**
 * ================================================================
 * ESP-SparkBot -- 输入处理实现 (触摸 + IMU 手势)
 * ESP-SparkBot -- Input Handler Implementation (Touch + IMU)
 * ================================================================
 *
 * 替代原来的 src/interaction.py
 * Replaces the original src/interaction.py
 *
 * 实现:
 *   - CST816T 电容触摸 I2C 通信
 *   - BMI270 六轴 IMU I2C 通信
 *   - 自定义手势识别算法
 *   - 2D 物理模拟 (弹跳/抛掷)
 *
 * Implements:
 *   - CST816T capacitive touch I2C communication
 *   - BMI270 6-axis IMU I2C communication
 *   - Custom gesture recognition algorithm
 *   - 2D physics simulation (bounce/fling)
 */

#include "touch_motion.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <math.h>

static const char *TAG = "input";

/* ================================================================
 * I2C 辅助函数 | I2C Helper Functions
 * ================================================================ */

/**
 * @brief 向 I2C 设备写入一个字节
 * Write a single byte to an I2C device
 */
static esp_err_t i2c_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
    uint8_t buf[2] = {reg_addr, data};
    return i2c_master_write_to_device(
        CONFIG_SPARKBOT_IMU_I2C_PORT,
        dev_addr,
        buf, sizeof(buf),
        pdMS_TO_TICKS(100)
    );
}

/**
 * @brief 从 I2C 设备读取多个字节
 * Read multiple bytes from an I2C device
 */
static esp_err_t i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr,
                                 uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(
        CONFIG_SPARKBOT_IMU_I2C_PORT,
        dev_addr,
        &reg_addr, 1,
        data, len,
        pdMS_TO_TICKS(100)
    );
}

/* ================================================================
 * BMI270 寄存器定义 | BMI270 Register Definitions
 * ================================================================ */

#define BMI270_I2C_ADDR         0x68    /* BMI270 默认 I2C 地址 (SDO 拉低) */
#define BMI270_CHIP_ID_REG      0x00    /* 芯片 ID 寄存器 (应为 0x24) */
#define BMI270_CMD_REG          0x7E    /* 命令寄存器 */
#define BMI270_PWR_CTRL_REG     0x7D    /* 电源控制寄存器 */
#define BMI270_ACC_DATA_REG     0x0C    /* 加速度数据起始寄存器 */
#define BMI270_GYR_DATA_REG     0x12    /* 陀螺仪数据起始寄存器 */

#define BMI270_CHIP_ID          0x24    /* BMI270 预期芯片 ID */
#define BMI270_CMD_SOFT_RESET   0xB6    /* 软复位命令 */
#define BMI270_CMD_ACC_NORMAL   0x40    /* 加速度计正常模式命令 */

/* ================================================================
 * CST816T 寄存器定义 | CST816T Register Definitions
 * ================================================================ */

#define CST816T_I2C_ADDR        0x15    /* CST816T I2C 地址 */
#define CST816T_GESTURE_REG     0x01    /* 手势寄存器 */
#define CST816T_XH_REG          0x03    /* X 坐标高字节 */
#define CST816T_XL_REG          0x04    /* X 坐标低字节 */
#define CST816T_YH_REG          0x05    /* Y 坐标高字节 */
#define CST816T_YL_REG          0x06    /* Y 坐标低字节 */
#define CST816T_POINTS_REG      0x02    /* 触摸点数寄存器 */

/* ================================================================
 * 手势识别参数 | Gesture Recognition Parameters
 * ================================================================ */

#define GESTURE_HISTORY_SIZE    32      /* IMU 历史数据缓冲区大小 | IMU history buffer size */
#define SHAKE_THRESHOLD         800     /* 摇晃加速度阈值 (mG) | Shake accel threshold */
#define DOUBLE_TAP_THRESHOLD    1200    /* 双击加速度阈值 (mG) | Double-tap accel threshold */
#define TILT_THRESHOLD          300     /* 倾斜检测阈值 (mG) | Tilt detection threshold */
#define DOUBLE_TAP_MIN_INTERVAL 100000  /* 双击最小间隔 (us) | Double-tap min interval */
#define DOUBLE_TAP_MAX_INTERVAL 500000  /* 双击最大间隔 (us) | Double-tap max interval */

/* IMU 历史数据环形缓冲区 | IMU history ring buffer */
static imu_data_t s_imu_history[GESTURE_HISTORY_SIZE];
static uint8_t    s_imu_history_idx = 0;
static uint8_t    s_imu_history_count = 0;

/* 双击检测状态 | Double-tap detection state */
static int64_t s_last_tap_time = 0;
static bool    s_tap_pending = false;

/* ================================================================
 * BMI270 实现 | BMI270 Implementation
 * ================================================================ */

esp_err_t touch_motion_imu_init(void)
{
    ESP_LOGI(TAG, "[BMI270] 正在初始化... | Initializing...");

    /* 1. 软复位 | Soft reset */
    esp_err_t ret = i2c_write_byte(BMI270_I2C_ADDR, BMI270_CMD_REG,
                                    BMI270_CMD_SOFT_RESET);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[BMI270] 软复位失败 | Soft reset failed: %s", esp_err_to_name(ret));
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(50));

    /* 2. 验证芯片 ID | Verify chip ID */
    uint8_t chip_id = 0;
    ret = i2c_read_bytes(BMI270_I2C_ADDR, BMI270_CHIP_ID_REG, &chip_id, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[BMI270] 读取芯片ID失败 | Read chip ID failed: %s", esp_err_to_name(ret));
        return ret;
    }

    if (chip_id != BMI270_CHIP_ID) {
        ESP_LOGW(TAG, "[BMI270] 芯片ID不匹配: 期望 0x%02X, 实际 0x%02X | "
                 "Chip ID mismatch: expected 0x%02X, got 0x%02X",
                 BMI270_CHIP_ID, chip_id);
        /* 继续运行 — 可能是兼容芯片 | Continue — might be compatible chip */
    }

    /* 3. 启用加速度计 (正常模式) | Enable accelerometer (normal mode) */
    ret = i2c_write_byte(BMI270_I2C_ADDR, BMI270_PWR_CTRL_REG, 0x0E);  /* accel + gyro enabled */
    vTaskDelay(pdMS_TO_TICKS(50));

    /* 4. 设置加速度计为正常模式 | Set accelerometer to normal mode */
    ret = i2c_write_byte(BMI270_I2C_ADDR, BMI270_CMD_REG, BMI270_CMD_ACC_NORMAL);
    vTaskDelay(pdMS_TO_TICKS(50));

    ESP_LOGI(TAG, "[BMI270] 初始化完成 (ChipID: 0x%02X) | Initialized OK", chip_id);

    /* 初始化手势历史缓冲区 | Initialize gesture history buffer */
    memset(s_imu_history, 0, sizeof(s_imu_history));
    s_imu_history_idx = 0;
    s_imu_history_count = 0;

    return ESP_OK;
}

esp_err_t touch_motion_read_imu(imu_data_t *data)
{
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buf[12];  /* 6 bytes accel + 6 bytes gyro */

    /* 读取加速度 + 陀螺仪数据 (12 字节) */
    /* Read accelerometer + gyroscope data (12 bytes) */
    esp_err_t ret = i2c_read_bytes(BMI270_I2C_ADDR, BMI270_ACC_DATA_REG,
                                    buf, sizeof(buf));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[BMI270] 读取数据失败 | Read data failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* 解析加速度数据 (小端序, 有符号16位) */
    /* Parse accelerometer data (little-endian, signed 16-bit) */
    data->accel_x = (int16_t)((buf[1] << 8) | buf[0]);
    data->accel_y = (int16_t)((buf[3] << 8) | buf[2]);
    data->accel_z = (int16_t)((buf[5] << 8) | buf[4]);

    /* 解析陀螺仪数据 (小端序, 有符号16位) */
    /* Parse gyroscope data (little-endian, signed 16-bit) */
    data->gyro_x = (int16_t)((buf[7]  << 8) | buf[6]);
    data->gyro_y = (int16_t)((buf[9]  << 8) | buf[8]);
    data->gyro_z = (int16_t)((buf[11] << 8) | buf[10]);

    data->timestamp = esp_timer_get_time();

    /* 存入历史缓冲区 | Store in history buffer */
    s_imu_history[s_imu_history_idx] = *data;
    s_imu_history_idx = (s_imu_history_idx + 1) % GESTURE_HISTORY_SIZE;
    if (s_imu_history_count < GESTURE_HISTORY_SIZE) {
        s_imu_history_count++;
    }

    return ESP_OK;
}

gesture_type_t touch_motion_detect_gesture(const imu_data_t *current_data)
{
    if (current_data == NULL) {
        return GESTURE_NONE;
    }

    /* ---- 1. 计算加速度矢量模值 | Calculate acceleration vector magnitude ---- */
    float accel_mag = sqrtf(
        (float)current_data->accel_x * current_data->accel_x +
        (float)current_data->accel_y * current_data->accel_y +
        (float)current_data->accel_z * current_data->accel_z
    );

    /* ---- 2. 自由落体检测 | Free fall detection ---- */
    if (accel_mag < 200) {  /* 接近 0G = 自由落体 | Near 0G = free fall */
        ESP_LOGI(TAG, "[手势] 检测到自由落体! | Free fall detected!");
        return GESTURE_FREE_FALL;
    }

    /* ---- 3. 双击检测 | Double-tap detection ---- */
    if (accel_mag > DOUBLE_TAP_THRESHOLD) {
        int64_t now = esp_timer_get_time();
        int64_t interval = now - s_last_tap_time;

        if (s_tap_pending &&
            interval >= DOUBLE_TAP_MIN_INTERVAL &&
            interval <= DOUBLE_TAP_MAX_INTERVAL) {
            /* 第二次敲击 — 双击! | Second tap — double-tap! */
            ESP_LOGI(TAG, "[手势] 检测到双击! | Double-tap detected!");
            s_tap_pending = false;
            s_last_tap_time = 0;
            return GESTURE_DOUBLE_TAP;
        }

        /* 记录第一次敲击 | Record first tap */
        s_last_tap_time = now;
        s_tap_pending = true;
    }

    /* ---- 4. 摇晃检测 | Shake detection ---- */
    if (accel_mag > SHAKE_THRESHOLD && s_imu_history_count >= 4) {
        /* 检查近几帧是否有方向突变 | Check recent frames for direction changes */
        uint8_t idx = (s_imu_history_idx - 1 + GESTURE_HISTORY_SIZE) % GESTURE_HISTORY_SIZE;
        int16_t prev_x = s_imu_history[idx].accel_x;

        if (abs(current_data->accel_x - prev_x) > SHAKE_THRESHOLD / 2) {
            ESP_LOGI(TAG, "[手势] 检测到摇晃! | Shake detected!");
            return GESTURE_SHAKE;
        }
    }

    /* ---- 5. 倾斜检测 | Tilt detection ---- */
    /* 静态加速度分量: 当设备静止时, 指向重力方向 */
    /* Static acceleration component: points to gravity when device is still */
    if (current_data->accel_x > TILT_THRESHOLD) {
        return GESTURE_TILT_RIGHT;
    }
    if (current_data->accel_x < -TILT_THRESHOLD) {
        return GESTURE_TILT_LEFT;
    }
    if (current_data->accel_y > TILT_THRESHOLD) {
        return GESTURE_TILT_FORWARD;
    }
    if (current_data->accel_y < -TILT_THRESHOLD) {
        return GESTURE_TILT_BACK;
    }

    /* ---- 6. 翻转检测 | Flip detection ---- */
    if (current_data->accel_z < -800) {  /* Z 轴为负 = 翻转 | Negative Z = flipped */
        ESP_LOGI(TAG, "[手势] 检测到翻转! | Flip detected!");
        return GESTURE_FLIP;
    }

    return GESTURE_NONE;
}

/* ================================================================
 * CST816T 触摸实现 | CST816T Touch Implementation
 * ================================================================ */

esp_err_t touch_motion_touch_init(void)
{
    ESP_LOGI(TAG, "[CST816T] 正在初始化触摸屏... | Initializing touch screen...");

    /* RST 脚复位 (需要硬件支持) 或通过 I2C 软复位 */
    /* RST pin reset (needs hardware support) or I2C soft reset */

    /* 验证设备存在 — 尝试读取手势寄存器 */
    /* Verify device existence — try reading gesture register */
    uint8_t gesture = 0;
    esp_err_t ret = i2c_read_bytes(CST816T_I2C_ADDR, CST816T_GESTURE_REG,
                                    &gesture, 1);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "[CST816T] 初始化失败, I2C通信错误 | Init failed, I2C comm error: %s",
                 esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "[CST816T] 触摸屏初始化完成, 手势寄存器: 0x%02X | "
             "Touch initialized, gesture reg: 0x%02X", gesture);
    return ESP_OK;
}

esp_err_t touch_motion_read_touch(touch_data_t *data)
{
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    /* 读取手势寄存器 | Read gesture register */
    uint8_t gesture = 0;
    esp_err_t ret = i2c_read_bytes(CST816T_I2C_ADDR, CST816T_GESTURE_REG,
                                    &gesture, 1);
    if (ret != ESP_OK) {
        return ret;
    }

    data->gesture = gesture;

    /* 读取触摸点数 | Read number of touch points */
    uint8_t points = 0;
    ret = i2c_read_bytes(CST816T_I2C_ADDR, CST816T_POINTS_REG, &points, 1);

    /* 有触摸时读取坐标 | Read coordinates when touched */
    if (gesture != TOUCH_GESTURE_NONE || points > 0) {
        data->touched = true;
        data->points = (points > 0) ? (points & 0x0F) : 1;

        /* 读取 X 坐标 (12-bit, 高4位在高字节, 低8位在低字节) */
        /* Read X coordinate (12-bit, upper 4 in high byte, lower 8 in low byte) */
        uint8_t xh = 0, xl = 0;
        i2c_read_bytes(CST816T_I2C_ADDR, CST816T_XH_REG, &xh, 1);
        i2c_read_bytes(CST816T_I2C_ADDR, CST816T_XL_REG, &xl, 1);
        data->x = (uint16_t)(((xh & 0x0F) << 8) | xl);

        /* 读取 Y 坐标 */
        /* Read Y coordinate */
        uint8_t yh = 0, yl = 0;
        i2c_read_bytes(CST816T_I2C_ADDR, CST816T_YH_REG, &yh, 1);
        i2c_read_bytes(CST816T_I2C_ADDR, CST816T_YL_REG, &yl, 1);
        data->y = (uint16_t)(((yh & 0x0F) << 8) | yl);

        ESP_LOGD(TAG, "[触摸] x=%d y=%d gesture=0x%02X | Touch x=%d y=%d gesture=0x%02X",
                 data->x, data->y, gesture);
        return ESP_OK;
    }

    data->touched = false;
    data->points  = 0;
    data->x = 0;
    data->y = 0;

    return ESP_ERR_NOT_FOUND;  /* 无触摸 | No touch */
}

/* ================================================================
 * 2D 物理模拟 | 2D Physics Simulation
 * ================================================================ */

void physics_body_init(physics_body_t *body, float x, float y)
{
    body->x      = x;
    body->y      = y;
    body->vx     = 0.0f;
    body->vy     = 0.0f;
    body->friction = 0.92f;
    body->bounce   = 0.5f;
    body->gravity  = 0.5f;
    body->use_gravity = false;
}

static void physics_boundary_check(physics_body_t *body,
                                    float screen_w, float screen_h,
                                    float obj_w, float obj_h)
{
    /* 左边界 | Left boundary */
    if (body->x < 0) {
        body->x  = 0;
        body->vx = fabsf(body->vx) * body->bounce;
    }
    /* 右边界 | Right boundary */
    else if (body->x > screen_w - obj_w) {
        body->x  = screen_w - obj_w;
        body->vx = -fabsf(body->vx) * body->bounce;
    }

    /* 上边界 | Top boundary */
    if (body->y < 0) {
        body->y  = 0;
        body->vy = fabsf(body->vy) * body->bounce;
    }

    /* 下边界 (地面) | Bottom boundary (ground) */
    if (body->y > screen_h - obj_h) {
        body->y  = screen_h - obj_h;
        body->vy = -fabsf(body->vy) * body->bounce;
        /* 弹跳速度很小时停止 | Stop if bounce velocity is too small */
        if (fabsf(body->vy) < 1.0f) {
            body->vy = 0;
        }
    }
}

void physics_body_update(physics_body_t *body,
                         float screen_width, float screen_height,
                         float obj_width, float obj_height)
{
    /* 应用重力 | Apply gravity */
    if (body->use_gravity) {
        body->vy += body->gravity;
    }

    /* 应用摩擦力 | Apply friction */
    body->vx *= body->friction;
    if (body->use_gravity) {
        body->vy *= body->friction;
    }

    /* 更新位置 | Update position */
    body->x += body->vx;
    body->y += body->vy;

    /* 边界碰撞检测 | Boundary collision check */
    physics_boundary_check(body, screen_width, screen_height, obj_width, obj_height);

    /* 速度过小时停止 (避免无限微小运动) */
    /* Stop when velocity is tiny (avoid infinite micro-motion) */
    if (fabsf(body->vx) < 0.1f) body->vx = 0.0f;
    if (fabsf(body->vy) < 0.1f && !body->use_gravity) body->vy = 0.0f;
}

void physics_body_fling(physics_body_t *body, float vx, float vy)
{
    body->vx = vx;
    body->vy = vy;
}

void physics_body_stop(physics_body_t *body)
{
    body->vx = 0.0f;
    body->vy = 0.0f;
}

bool physics_body_is_moving(const physics_body_t *body)
{
    return fabsf(body->vx) > 0.5f || fabsf(body->vy) > 0.5f;
}
