/**
 * ================================================================
 * ESP-SparkBot -- 输入处理组件 (触摸 + IMU 手势)
 * ESP-SparkBot -- Input Handler Component (Touch + IMU Gestures)
 * ================================================================
 *
 * 替代原来的 src/interaction.py
 * Replaces the original src/interaction.py
 *
 * 功能 (Functions):
 *   - CST816T 电容触摸屏读取 | CST816T capacitive touch reading
 *   - BMI270 六轴 IMU 数据读取 | BMI270 6-axis IMU data reading
 *   - 手势识别 (双击, 摇晃, 倾斜, 翻转) | Gesture recognition (double-tap, shake, tilt, flip)
 *   - 物理按键处理 | Physical button handling
 *   - 物理模拟 (可选 — 宠物抛掷效果) | Physics simulation (optional — pet fling effect)
 */

#ifndef TOUCH_MOTION_H
#define TOUCH_MOTION_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * 触摸数据结构 | Touch Data Structure
 * ================================================================ */

/**
 * @brief 触摸点数据
 * Touch Point Data
 */
typedef struct {
    bool     touched;   /* 是否正在触摸 | Is currently being touched */
    uint16_t x;         /* 触摸 X 坐标 | Touch X coordinate (0-239) */
    uint16_t y;         /* 触摸 Y 坐标 | Touch Y coordinate (0-239) */
    uint8_t  gesture;   /* 触摸手势类型 (CST816T 芯片支持) | Touch gesture type */
    uint8_t  points;    /* 同时触摸点数 | Number of simultaneous touch points */
} touch_data_t;

/* CST816T 手势类型 | CST816T gesture types */
#define TOUCH_GESTURE_NONE          0x00  /* 无手势 | No gesture */
#define TOUCH_GESTURE_SWIPE_UP      0x01  /* 上划 | Swipe up */
#define TOUCH_GESTURE_SWIPE_DOWN    0x02  /* 下划 | Swipe down */
#define TOUCH_GESTURE_SWIPE_LEFT    0x03  /* 左划 | Swipe left */
#define TOUCH_GESTURE_SWIPE_RIGHT   0x04  /* 右划 | Swipe right */
#define TOUCH_GESTURE_SINGLE_CLICK  0x05  /* 单击 | Single click */
#define TOUCH_GESTURE_DOUBLE_CLICK  0x0B  /* 双击 | Double click */
#define TOUCH_GESTURE_LONG_PRESS    0x0C  /* 长按 | Long press */

/* ================================================================
 * IMU 数据结构 | IMU Data Structure
 * ================================================================ */

/**
 * @brief BMI270 IMU 数据
 * BMI270 IMU Data
 */
typedef struct {
    /* 加速度计 (mG, 1G = 1000) */
    /* Accelerometer (milli-G, 1G = 1000) */
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;

    /* 陀螺仪 (mDPS, 即 毫度/秒) */
    /* Gyroscope (milli-DPS = milli-degrees-per-second) */
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;

    int64_t timestamp;  /* 数据时间戳 (us) | Data timestamp */
} imu_data_t;

/* ================================================================
 * 手势识别结果 | Gesture Recognition Results
 * ================================================================ */

/**
 * @brief 手势类型枚举
 * Gesture Type Enumeration
 *
 * 基于 BMI270 加速度/陀螺仪数据的高级手势识别。
 * High-level gesture recognition based on BMI270 accel/gyro data.
 */
typedef enum {
    GESTURE_NONE        = 0,  /* 无手势 | No gesture */
    GESTURE_DOUBLE_TAP  = 1,  /* 双击 — 快速两次敲击 | Double-tap — two quick knocks */
    GESTURE_SHAKE       = 2,  /* 摇晃 — 水平快速摆动 | Shake — rapid horizontal swing */
    GESTURE_TILT_LEFT   = 3,  /* 向左倾斜 | Tilt left */
    GESTURE_TILT_RIGHT  = 4,  /* 向右倾斜 | Tilt right */
    GESTURE_TILT_FORWARD= 5,  /* 向前倾斜 | Tilt forward */
    GESTURE_TILT_BACK   = 6,  /* 向后倾斜 | Tilt back */
    GESTURE_FREE_FALL   = 7,  /* 自由落体 | Free fall */
    GESTURE_FLIP        = 8,  /* 翻转 (上下颠倒) | Flip (upside down) */
    GESTURE_WAKE_UP     = 9,  /* 唤醒 — 检测到拿起动作 | Wake-up — pick-up detected */
} gesture_type_t;

/* ================================================================
 * 物理模拟结构 (可选) | Physics Simulation Structure (Optional)
 * ================================================================ */

/**
 * @brief 2D 物理体 (用于宠物抛掷弹跳效果)
 * 2D Physics Body (for pet fling & bounce effect)
 *
 * 移植自原 Python 版本的 PhysicsBody 类。
 * Ported from the original Python PhysicsBody class.
 */
typedef struct {
    float x;              /* 当前位置 X | Current position X */
    float y;              /* 当前位置 Y | Current position Y */
    float vx;             /* 水平速度 | Horizontal velocity */
    float vy;             /* 垂直速度 | Vertical velocity */
    float friction;       /* 摩擦力系数 | Friction coefficient (0.0-1.0) */
    float bounce;         /* 弹跳系数 | Bounce coefficient (0.0-1.0) */
    float gravity;        /* 重力加速度 | Gravity acceleration */
    bool  use_gravity;    /* 是否启用重力 | Gravity enabled */
} physics_body_t;

/* ================================================================
 * 公共 API | Public API
 * ================================================================ */

/**
 * @brief 初始化 BMI270 IMU 传感器
 * Initialize BMI270 IMU Sensor
 *
 * 通过 I2C 总线 (引脚定义在 sdkconfig.defaults 中) 连接 BMI270。
 * Connects to BMI270 via I2C bus (pins defined in sdkconfig.defaults).
 *
 * @return ESP_OK 成功 | ESP_OK on success
 */
esp_err_t touch_motion_imu_init(void);

/**
 * @brief 初始化 CST816T 电容触摸屏
 * Initialize CST816T Capacitive Touch Screen
 *
 * @return ESP_OK 成功 | ESP_OK on success
 */
esp_err_t touch_motion_touch_init(void);

/**
 * @brief 读取触摸数据
 * Read Touch Data
 *
 * @param[out] data 触摸数据结构指针 | Pointer to touch data struct
 * @return ESP_OK 成功, ESP_ERR_NOT_FOUND 无触摸 | ESP_OK on success, ESP_ERR_NOT_FOUND if no touch
 */
esp_err_t touch_motion_read_touch(touch_data_t *data);

/**
 * @brief 读取 IMU 数据
 * Read IMU Data
 *
 * @param[out] data IMU 数据结构指针 | Pointer to IMU data struct
 * @return ESP_OK 成功 | ESP_OK on success
 */
esp_err_t touch_motion_read_imu(imu_data_t *data);

/**
 * @brief 执行手势识别算法
 * Execute Gesture Recognition Algorithm
 *
 * 基于 IMU 数据的历史缓冲区检测手势。
 * Detects gestures based on IMU data history buffer.
 *
 * @param current_data 当前帧的 IMU 数据 | Current frame IMU data
 * @return 检测到的手势类型 | Detected gesture type
 */
gesture_type_t touch_motion_detect_gesture(const imu_data_t *current_data);

/**
 * @brief 物理体初始化
 * Initialize Physics Body
 *
 * @param body 物理体指针 | Pointer to physics body
 * @param x    初始 X 坐标 | Initial X coordinate
 * @param y    初始 Y 坐标 | Initial Y coordinate
 */
void physics_body_init(physics_body_t *body, float x, float y);

/**
 * @brief 物理体状态更新 (每帧调用)
 * Update Physics Body State (called every frame)
 *
 * @param body          物理体指针 | Pointer to physics body
 * @param screen_width  屏幕宽度 | Screen width
 * @param screen_height 屏幕高度 | Screen height
 * @param obj_width     物体宽度 | Object width
 * @param obj_height    物体高度 | Object height
 */
void physics_body_update(physics_body_t *body,
                         float screen_width, float screen_height,
                         float obj_width, float obj_height);

/**
 * @brief 对物理体施加抛掷力
 * Apply Fling Force to Physics Body
 *
 * @param body 物理体指针 | Pointer to physics body
 * @param vx   水平初速度 | Horizontal initial velocity
 * @param vy   垂直初速度 | Vertical initial velocity
 */
void physics_body_fling(physics_body_t *body, float vx, float vy);

/**
 * @brief 停止物理体运动
 * Stop Physics Body Motion
 */
void physics_body_stop(physics_body_t *body);

/**
 * @brief 物理体是否在运动
 * Check if Physics Body is Moving
 *
 * @return true 运动中 | Moving
 * @return false 静止 | Stationary
 */
bool physics_body_is_moving(const physics_body_t *body);

#ifdef __cplusplus
}
#endif

#endif /* TOUCH_MOTION_H */
