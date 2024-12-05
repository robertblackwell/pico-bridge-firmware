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
set(HASH_CMD ${CMAKE_CURRENT_SOURCE_DIR}/git_hash.py)
execute_process(COMMAND ${HASH_CMD} OUTPUT_VARIABLE BRANCH_HASH  ECHO_OUTPUT_VARIABLE )
message("hash command ${HASH_CMD}")
MESSAGE("BRANCH_HASH  [${BRANCH_HASH}]")
# Initialize the SDK

pico_sdk_init()

set(COMMON_SOURCE_FILES 
    ${COMMON_SRC_DIR}/cli/argv.cpp
    ${COMMON_SRC_DIR}/cli/argv.h
    ${COMMON_SRC_DIR}/cli/commands.h
    ${COMMON_SRC_DIR}/cli/commands.cpp
    ${COMMON_SRC_DIR}/cli/execute_commands.cpp

    ${COMMON_SRC_DIR}/enum.cpp
    ${COMMON_SRC_DIR}/enum.h
    ${COMMON_SRC_DIR}/trace.cpp
    ${COMMON_SRC_DIR}/trace.h
    ${COMMON_SRC_DIR}/dri0002.h
    ${COMMON_SRC_DIR}/dri0002.cpp
    ${COMMON_SRC_DIR}/encoder.h
    ${COMMON_SRC_DIR}/encoder.cpp
    ${COMMON_SRC_DIR}/encoder_sample.h
    ${COMMON_SRC_DIR}/encoder_sample.cpp
    ${COMMON_SRC_DIR}/pico_gpio_irq_dispatcher.h
    ${COMMON_SRC_DIR}/pico_gpio_irq_dispatcher.cpp
    ${COMMON_SRC_DIR}/motion.h
    ${COMMON_SRC_DIR}/motion.cpp
    ${COMMON_SRC_DIR}/robot.h
    ${COMMON_SRC_DIR}/robot.cpp
    ${COMMON_SRC_DIR}/task.h

    ${COMMON_SRC_DIR}/transport/buffers.h
    ${COMMON_SRC_DIR}/transport/buffers.cpp
    ${COMMON_SRC_DIR}/transport/transmit_buffer_pool.h
    ${COMMON_SRC_DIR}/transport/transmit_buffer_pool.cpp
    ${COMMON_SRC_DIR}/transport/transport.h
    ${COMMON_SRC_DIR}/transport/transport.cpp
    ${COMMON_SRC_DIR}/transport/transport_reader.cpp

    ${COMMON_SRC_DIR}/pid.h
    ${COMMON_SRC_DIR}/pid.cpp
)

set(PICO_ROBOT_PATH ${PROJECT_SOURCE_DIR})

add_subdirectory(apps/debug-blinky)
add_subdirectory(apps/test01)
add_subdirectory(apps/test_command)
add_subdirectory(apps/test_uart)
add_subdirectory(apps/test_serial_ports)
add_subdirectory(apps/helloworld)
# add_subdirectory(apps/test_cli)
add_subdirectory(apps/test_irq)
# add_subdirectory(apps/test_motor)
# add_subdirectory(apps/test_motion)
add_subdirectory(apps/bridge)
add_subdirectory(apps/closed_loop)
add_subdirectory(apps/test_transport)
# add_subdirectory(apps/onboard_pid)

