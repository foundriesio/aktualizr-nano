#Description: aktualizr-nano tests; user_visible: False
include_guard(GLOBAL)
message("middleware_aktualizr-nano_tests component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/test/aknano_test.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/test/
)
