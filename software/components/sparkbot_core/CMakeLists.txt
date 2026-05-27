# SparkBot Core Component CMakeLists
# 核心组件 CMakeLists — 硬件抽象层与系统管理
# Core component CMakeLists — HAL and system management

idf_component_register(
    SRCS
        "sparkbot_core.c"
    INCLUDE_DIRS
        "."
    REQUIRES
        driver
        esp_wifi
        esp_netif
        esp_event
        esp_camera
        nvs_flash
)

# SparkBot Core 是基础组件 — 不依赖其他自定义组件
# SparkBot Core is a foundational component — doesn't depend on other custom components
