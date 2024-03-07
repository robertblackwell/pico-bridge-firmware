cmake_minimum_required(VERSION 3.27)

set(ENV{PICO_SDK_PATH} /home/robert/pico/pico/pico-sdk)
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
message("NOW")
message("PICO_SDK_PATH         ${PICO_SDK_PATH}")
message("env(PICO_SDK_PATH)    $ENV{PICO_SDK_PATH}")
message("NOW")
set(COMMON_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src/common)
message("COMMON_SRC_DIR    ${COMMON_SRC_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
include(pico_sdk_import.cmake)
# optional
# include(pico_extras_import_optional.cmake)
if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(pico_robot LANGUAGES C CXX ASM VERSION 0.0.1)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# set(PIC_CXX_ENABLE_EXCEPTIONS 1)

# Initialize the SDK
pico_sdk_init()

set(PICO_ROBOT_PATH ${PROJECT_SOURCE_DIR})

add_subdirectory(src/debug-blinky)
add_subdirectory(src/test01)
add_subdirectory(src/test_command)
add_subdirectory(src/test_uart)
add_subdirectory(src/test_serial_ports)
add_subdirectory(src/helloworld)
# add_subdirectory(src/test_cli)
add_subdirectory(src/test_irq)
# add_subdirectory(src/test_motor)
# add_subdirectory(src/test_motion)
add_subdirectory(src/bridge)
add_subdirectory(src/test_transport)
# add_subdirectory(src/onboard_pid)

