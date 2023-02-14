#Description: aktualizr-nano; user_visible: False
include_guard(GLOBAL)
message("middleware_aktualizr-nano component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/aknano_api.c
    ${CMAKE_CURRENT_LIST_DIR}/src/aknano_device_gateway.c
    ${CMAKE_CURRENT_LIST_DIR}/src/aknano_targets_manifest.c
    ${CMAKE_CURRENT_LIST_DIR}/src/aknano_flash_storage.c
    ${CMAKE_CURRENT_LIST_DIR}/src/aknano_image_download.c
    ${CMAKE_CURRENT_LIST_DIR}/src/aknano_rest_api.c
    ${CMAKE_CURRENT_LIST_DIR}/src/aknano_tuf_client.c
    ${CMAKE_CURRENT_LIST_DIR}/src/aknano.c
    ${CMAKE_CURRENT_LIST_DIR}/src/aknano_net.c
    ${CMAKE_CURRENT_LIST_DIR}/src/aknano_pkcs11.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
)
