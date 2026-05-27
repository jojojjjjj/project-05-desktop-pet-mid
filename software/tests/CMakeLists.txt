# ESP-SparkBot Unit Tests CMakeLists
# ESP-SparkBot 单元测试 CMakeLists

# ESP-IDF unit test component
idf_component_register(
    SRCS
        "test_basic.c"
    INCLUDE_DIRS
        "."
        "../main"
        "../components/sparkbot_core"
        "../components/display"
        "../components/input"
        "../components/ai"
    REQUIRES
        unity
        cmock
)
