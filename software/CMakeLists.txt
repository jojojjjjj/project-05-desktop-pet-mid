# ================================================================
# ESP-SparkBot 项目级 CMakeLists
# ESP-SparkBot Top-Level CMakeLists
# ================================================================
# ESP-IDF 项目构建入口，定义项目名称、依赖组件和全局编译选项
# ESP-IDF project build entry, defines project name, component
# dependencies, and global compile options.

cmake_minimum_required(VERSION 3.16)

# ---- 项目基本信息 (Project Metadata) ----
# ESP-IDF 使用 project() 自动包含 ESP-IDF 构建系统
set(PROJECT_NAME "esp_sparkbot")
set(PROJECT_VER  "1.0.0")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(${PROJECT_NAME})

# ---- 全局编译选项 (Global Compile Flags) ----
# 开启 C11 标准，优化flash内存使用
# Enable C11 standard, optimize flash usage
add_compile_options(
    -std=gnu11
    -Os
    -ffunction-sections
    -fdata-sections
    -Wno-unused-parameter
    -Wno-missing-field-initializers
)

# ---- 自定义组件目录 (Custom Component Directories) ----
# EXTRA_COMPONENT_DIRS 告诉 ESP-IDF 哪里搜索额外的组件
# Tells ESP-IDF where to search for extra components
set(EXTRA_COMPONENT_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/components
    ${CMAKE_CURRENT_SOURCE_DIR}/external_components
)

# ---- 如果存在 external_components 目录则包含 (Include if exists) ----
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external_components)
    message(STATUS "[SparkBot] 找到外部组件目录 | Found external components")
endif()

message(STATUS "[SparkBot] 项目已配置 | Project configured: ${PROJECT_NAME} v${PROJECT_VER}")
