# Display Component CMakeLists
# 显示屏组件 CMakeLists — LVGL 显示与动画管理
# Display component CMakeLists — LVGL display & animation management

idf_component_register(
    SRCS
        "lvgl_display.c"
    INCLUDE_DIRS
        "."
    REQUIRES
        lvgl
        driver
        esp_lcd
)
