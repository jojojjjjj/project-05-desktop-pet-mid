# Main Component CMakeLists
# 主组件 CMakeLists - 编译 main/ 目录下的所有 C 源文件
# Compiles all C source files under the main/ directory

idf_component_register(
    SRCS
        "app_main.c"
        "state_machine.c"
        "utils.c"
    INCLUDE_DIRS
        "."
        "../components/sparkbot_core"
        "../components/display"
        "../components/input"
        "../components/ai"
    REQUIRES
        # 依赖的 ESP-IDF 组件 | Required ESP-IDF components
        nvs_flash
        esp_wifi
        esp_netif
        esp_event
        esp_http_client
        esp_camera
        esp_lcd
        driver
        lvgl
        json
    PRIV_REQUIRES
        sparkbot_core
)

# 如果 LVGL 不可用则跳过 display 组件
# Skip display component if LVGL is not available
