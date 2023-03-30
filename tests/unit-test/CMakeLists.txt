include( ${MODULE_ROOT_DIR}/aknano_file_paths.cmake )
include( ${LIBTUFNANO_DIR}/tufFilePaths.cmake )

# ====================  Define your project name (edit) ========================
set(project_name "aknano")

# =====================  Create your mock here  (edit)  ========================
# =====================  Create your mock here  (edit)  ========================

# list the files to mock here
    list(APPEND mock_list
        ${MODULE_ROOT_DIR}/src/aknano_net.h
        ${MODULE_ROOT_DIR}/src/aknano_flash_storage.h
    )
# list the directories your mocks need
    list(APPEND mock_include_list
        ${MODULE_ROOT_DIR}/tests/unit-test/CMock/src
        ${MODULE_ROOT_DIR}/tests/unit-test/Unity/src
        ${MBEDTLS_DIR}/include/
        ${MODULE_ROOT_DIR}/src/
        ${MODULE_ROOT_DIR}/src/platform/linux_tests
        # ${CMAKE_CURRENT_LIST_DIR}
        # ${abstraction_dir}/ble_hal/include
        # ${abstraction_dir}/platform/include
        # ${abstraction_dir}/platform/freertos/include
        # ${abstraction_dir}/platform/include
        # "${AFR_ROOT_DIR}/libraries/c_sdk/standard/common/include"
    )
#list the definitions of your mocks to control what to be included
    # list(APPEND mock_define_list
	# 	portUSING_MPU_WRAPPERS=1
    #     MPU_WRAPPERS_INCLUDED_FROM_API_FILE
    #     portHAS_STACK_OVERFLOW_CHECKING=1
    # )

# ================= Create the library under test here (edit) ==================

# Base name for temporary files
set( TEMP_BASE ${CMAKE_BINARY_DIR}/${project_name} )

# # Strip static constraints so unit tests may call internal functions
# execute_process( COMMAND sed "s/^static //"
#                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#                  INPUT_FILE ${TUF_SOURCES}
#                  OUTPUT_FILE ${TEMP_BASE}.c
#         )

# # Generate a header file for internal functions
# execute_process( COMMAND sed -n "/^static.*(/,/^{\$/{s/^static //; s/)\$/&;/; /{/d; p;}"
#                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#                  INPUT_FILE ${TUF_SOURCES}
#                  OUTPUT_FILE ${TEMP_BASE}_annex.h
# )

# list the files you would like to test here
list(APPEND real_source_files
            ${AKNANO_SOURCES}
            ${TUF_SOURCES}
            ${TUF_FIAT_SOURCES}
            ${JSON_SOURCES}
        )
# list the directories the module under test includes
list(APPEND real_include_directories
            ${AKNANO_INCLUDE_PUBLIC_DIRS}
            ${TUF_INCLUDE_PUBLIC_DIRS}
            ${JSON_INCLUDE_PUBLIC_DIRS}
            ${MBEDTLS_INCLUDE_DIRS}
            ${TUF_SAMPLE_CLIENT_INCLUDE_PUBLIC_DIRS}
        )

# =====================  Create UnitTest Code here (edit)  =====================

# list the directories your test needs to include
list(APPEND test_include_directories
            ${AKNANO_INCLUDE_PUBLIC_DIRS}
            ${TUF_INCLUDE_PUBLIC_DIRS}
            ${MBEDTLS_INCLUDE_DIRS}
            ${JSON_INCLUDE_PUBLIC_DIRS}
            ${UNIT_TEST_DIR}
            ${CMAKE_BINARY_DIR}
            ${TUF_SAMPLE_CLIENT_INCLUDE_PUBLIC_DIRS}
        )

# =============================  (end edit)  ===================================

set(real_name "${project_name}_real")
set(mock_name "${project_name}_mock")

create_mock_list(${mock_name}
            "${mock_list}"
            "${CMAKE_SOURCE_DIR}/../tools/cmock/project.yml"
            "${mock_include_list}"
            "${mock_define_list}"
        )

create_real_library(${real_name}
                    "${real_source_files}"
                    "${real_include_directories}"
                    "${mock_name}"
        )

list(APPEND utest_link_list
            -l${mock_name}
            lib${real_name}.a
        )

list(APPEND utest_dep_list
            ${real_name}
            mbedtls
            libtufnano_client
        )

set(utest_name "${project_name}_utest")
set(utest_source "${project_name}_utest.c")
create_test(${utest_name}
            ${utest_source}
            "${utest_link_list}"
            "${utest_dep_list}"
            "${test_include_directories}"
        )
