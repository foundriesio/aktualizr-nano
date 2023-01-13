#Description: aktualizr-nano EdgeLock 2GO; user_visible: False
include_guard(GLOBAL)
message("middleware_aktualizr-nano_el2go component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/aknano_el2go.c
)
