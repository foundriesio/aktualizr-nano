
# This file is to add source files and include directories
# into variables so that it can be reused from different repositories
# in their Cmake based build system by including this file.
#
# Files specific to the repository such as test runner, platform tests
# are not added to the variables.

# Aknano library source files.
set( AKNANO_SOURCES
     ${CMAKE_CURRENT_LIST_DIR}/src/platform/linux_tests/aknano_platform.c
     ${CMAKE_CURRENT_LIST_DIR}/src/aknano_targets_manifest.c
#     ${CMAKE_CURRENT_LIST_DIR}/src/aknano_api.c
#     ${CMAKE_CURRENT_LIST_DIR}/src/aknano_debug.c
#     ${CMAKE_CURRENT_LIST_DIR}/src/aknano_device_gateway.c
#     ${CMAKE_CURRENT_LIST_DIR}/src/aknano_flash_storage.c
#     ${CMAKE_CURRENT_LIST_DIR}/src/aknano_image_download.c
#     ${CMAKE_CURRENT_LIST_DIR}/src/aknano_rest_api.c
#     ${CMAKE_CURRENT_LIST_DIR}/src/aknano_tuf_client.c
#     ${CMAKE_CURRENT_LIST_DIR}/src/aknano.c
#     ${CMAKE_CURRENT_LIST_DIR}/src/aknano_net.c
#     ${CMAKE_CURRENT_LIST_DIR}/src/aknano_pkcs11.c
)     

# Aknano library Public Include directories.
set( AKNANO_INCLUDE_PUBLIC_DIRS
     ${CMAKE_CURRENT_LIST_DIR}/src 
     ${CMAKE_CURRENT_LIST_DIR}/src/platform/linux_tests
     )
