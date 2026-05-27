# Input Component CMakeLists
# 输入组件 CMakeLists — 触摸 + IMU + 按键
# Input component CMakeLists — Touch + IMU + Button

idf_component_register(
    SRCS
        "touch_motion.c"
    INCLUDE_DIRS
        "."
    REQUIRES
        driver
)
