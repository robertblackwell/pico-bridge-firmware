cmake_minimum_required(VERSION 3.27)

project(robot_linux_tests VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(PICO_ON_DEVICE)
    message(WARNING "Host tests included in a device build. These targets will be defined but may fail to build natively. To build them for host, use a separate CMake profile or build from tests_on_hosts directory.")
    # In a device build, we might not have Threads, but we still want the IDE to see the files.
    find_package(Threads)
else()
    # Find necessary packages
    find_package(Threads REQUIRED)
endif()

# Options
option(LOCAL_TESTS_Verbose "Print diagnostic stuff" ON)

# Directory Structure Calculations
set(TESTS_DIR ${CMAKE_CURRENT_LIST_DIR})
set(TESTS_COMMON_DIR ${TESTS_DIR}/common)

# Firmware directories
cmake_path(GET TESTS_DIR PARENT_PATH FIRMWARE_DIR)
set(FIRMWARE_SRC_DIR ${FIRMWARE_DIR}/src)
set(FIRMWARE_COMMON_DIR ${FIRMWARE_SRC_DIR}/common)

# FIRMWARE_DIR and FIRMWARE_COMMON_DIR are already calculated
add_subdirectory(${FIRMWARE_COMMON_DIR} src/common)

# Common configuration for all tests
include(CTest)
enable_testing()

# Add test subdirectories
add_subdirectory(test_wheel_speed)
# add_subdirectory(test_static_buffer_cpp)
# add_subdirectory(test_with_pico)
