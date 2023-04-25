# Copyright (C) 2023 Intel Corporation
# SPDX-License-Identifier: MIT

set(DUMMY_LIB_DIR ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/adapter-reg)

if(MSVC)
    set(DUMMY_LIB_DIR ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/adapter-reg)
else()
    set(DUMMY_LIB_DIR ${CMAKE_BINARY_DIR}/bin/adapter-reg)
endif()

if(WIN32)
    set(DUMMY_LIB_PATH ${DUMMY_LIB_DIR}/ur_adapter_level_zero.dll)
else()
    set(DUMMY_LIB_PATH ${DUMMY_LIB_DIR}/libur_adapter_level_zero.so.0)
endif()

# Create a dummy adapter library in adapter the runtime output directory
file(WRITE ${DUMMY_LIB_PATH} "")

function(add_adapter_reg_test name)
    cmake_parse_arguments(TEST "" "" "ENVS;SOURCES" ${ARGN})

    set(TEST_TARGET_NAME adapter-reg-test-${name})
    add_executable(${TEST_TARGET_NAME}
        ${TEST_SOURCES})
    if(WIN32)
        target_sources(${TEST_TARGET_NAME} PRIVATE
            ${CMAKE_SOURCE_DIR}/source/loader/windows/loader_location.cpp)
    else()
        target_sources(${TEST_TARGET_NAME} PRIVATE
            ${CMAKE_SOURCE_DIR}/source/loader/linux/loader_location.cpp)
    endif()
    target_link_libraries(${TEST_TARGET_NAME}
        PRIVATE
        ${PROJECT_NAME}::headers
        ${PROJECT_NAME}::common
        GTest::gtest_main)
    target_include_directories(${TEST_TARGET_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/source/loader)
    add_test(NAME adapter-reg-${name}
        COMMAND ${TEST_TARGET_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set_tests_properties(adapter-reg-${name} PROPERTIES LABELS "adapter-reg")
    set_property(TEST adapter-reg-${name} PROPERTY
        ENVIRONMENT ${TEST_ENVS})
endfunction()

function(add_adapter_reg_test_with_lib_in_same_dir name)
    cmake_parse_arguments(TEST "" "" "ENVS;SOURCES" ${ARGN})
    add_adapter_reg_test(${name}
        ENVS ${TEST_ENVS}
        SOURCES ${TEST_SOURCES})
    set_target_properties(adapter-reg-test-${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/adapter-reg)
endfunction()

add_adapter_reg_test(search-with-env
    ENVS "LD_LIBRARY_PATH=\"\"" "PATH=\"\"" "UR_ADAPTERS_SEARCH_PATH=\"${DUMMY_LIB_DIR}\""
    SOURCES search_success.cpp)
add_adapter_reg_test(search-with-sys-env
    ENVS "LD_LIBRARY_PATH=${DUMMY_LIB_DIR}" "PATH=${DUMMY_LIB_DIR}" "UR_ADAPTERS_SEARCH_PATH="
    SOURCES search_success.cpp)
add_adapter_reg_test(search-no-env
    ENVS "LD_LIBRARY_PATH=\"\"" "PATH=\"\"" "UR_ADAPTERS_SEARCH_PATH=\"\""
    SOURCES search_fail.cpp)
add_adapter_reg_test_with_lib_in_same_dir(search-same-dir
    ENVS "LD_LIBRARY_PATH=\"\"" "PATH=\"\"" "UR_ADAPTERS_SEARCH_PATH=\"\""
    SOURCES search_success.cpp)
