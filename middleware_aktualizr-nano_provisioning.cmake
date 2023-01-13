#Description: aktualizr-nano; user_visible: False
include_guard(GLOBAL)
message("middleware_aktualizr-nano component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/provisioning/aknano_provisioning_mbedtls.c
    ${CMAKE_CURRENT_LIST_DIR}/src/provisioning/aknano_provisioning.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/provisioning
)
