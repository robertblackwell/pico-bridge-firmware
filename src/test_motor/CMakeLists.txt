cmake_minimum_required(VERSION 3.13)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
include(pico_sdk_import.cmake)
project(test_project)
pico_sdk_init()
set(COMMON_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/../common)
message("COMMON_SRC_DIR    ${COMMON_SRC_DIR}")

add_executable(motor_test 
    motor_test.cpp
    ${COMMON_SRC_DIR}/utils.cpp
    ${COMMON_SRC_DIR}/utils.h
    ${COMMON_SRC_DIR}/dri0002.h
    ${COMMON_SRC_DIR}/dri0002.cpp
    )
target_include_directories(motor_test PRIVATE ${COMMON_SRC_DIR})
pico_enable_stdio_usb(motor_test 1)
pico_enable_stdio_uart(motor_test 0)
pico_add_extra_outputs(motor_test)
target_link_libraries(motor_test pico_stdlib hardware_gpio hardware_pwm)