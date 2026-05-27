# AI Component CMakeLists
# AI 组件 CMakeLists — LLM 对话管理
# AI component CMakeLists — LLM Dialog Manager

idf_component_register(
    SRCS
        "llm_dialog.c"
    INCLUDE_DIRS
        "."
    REQUIRES
        esp_http_client
        json
)
