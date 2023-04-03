
# This file is to add source files and include directories
# into variables so that it can be reused from different repositories
# in their Cmake based build system by including this file.
#
# Files specific to the repository such as test runner, platform tests
# are not added to the variables.

include(${CMAKE_CURRENT_LIST_DIR}/tests/unit-test/backoffAlgorithm/backoffAlgorithmFilePaths.cmake)

# Aknano library source files.
set( AKNANO_SOURCES
     ${CMAKE_CURRENT_LIST_DIR}/src/aknano_targets_manifest.c
     ${CMAKE_CURRENT_LIST_DIR}/src/aknano_device_gateway.c
     ${CMAKE_CURRENT_LIST_DIR}/src/aknano_api.c
     ${CMAKE_CURRENT_LIST_DIR}/src/aknano_image_download.c
     ${CMAKE_CURRENT_LIST_DIR}/src/aknano_tuf_client.c
     ${CMAKE_CURRENT_LIST_DIR}/src/aknano.c
     ${CMAKE_CURRENT_LIST_DIR}/tests/aknano_test_client/aknano_client.c

     ${BACKOFF_ALGORITHM_SOURCES}
     ${CMAKE_CURRENT_LIST_DIR}/tests/unit-test/inih/ini.c
)

# Aknano library Public Include directories.
set( AKNANO_INCLUDE_PUBLIC_DIRS
     ${CMAKE_CURRENT_LIST_DIR}/src
     ${CMAKE_CURRENT_LIST_DIR}/src/provisioning
     ${CMAKE_CURRENT_LIST_DIR}/src/platform/linux_tests
     ${BACKOFF_ALGORITHM_INCLUDE_PUBLIC_DIRS}
     ${CMAKE_CURRENT_LIST_DIR}/tests/unit-test/inih/
     )
