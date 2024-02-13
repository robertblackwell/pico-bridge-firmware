cmake_minimum_required(VERSION 3.13)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
include(pico_sdk_import.cmake)
project(test_project)
pico_sdk_init()
set(COMMON_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/../common)
message("COMMON_SRC_DIR    ${COMMON_SRC_DIR}")

add_executable(test_command 
    test_command.cpp
    # ${COMMON_SRC_DIR}/cli/argv.cpp
    # ${COMMON_SRC_DIR}/cli/argv.h
    # ${COMMON_SRC_DIR}/cli/commands.h
    # ${COMMON_SRC_DIR}/cli/commands.cpp

    # ${COMMON_SRC_DIR}/enum.cpp
    # ${COMMON_SRC_DIR}/enum.h
    ${COMMON_SRC_DIR}/trace.cpp
    ${COMMON_SRC_DIR}/trace.h
    # ${COMMON_SRC_DIR}/dri0002.h
    # ${COMMON_SRC_DIR}/dri0002.cpp
    # ${COMMON_SRC_DIR}/encoder.h
    # ${COMMON_SRC_DIR}/encoder.cpp
    # ${COMMON_SRC_DIR}/encoder_sample.h
    # ${COMMON_SRC_DIR}/encoder_sample.cpp
    # ${COMMON_SRC_DIR}/pico_gpio_irq_dispatcher.h
    # ${COMMON_SRC_DIR}/pico_gpio_irq_dispatcher.cpp
    # ${COMMON_SRC_DIR}/motion.h
    # ${COMMON_SRC_DIR}/motion.cpp
    # ${COMMON_SRC_DIR}/robot.h
    # ${COMMON_SRC_DIR}/robot.cpp
    # ${COMMON_SRC_DIR}/task.h

    # ${COMMON_SRC_DIR}/transport/buffers.h
    ${COMMON_SRC_DIR}/transport/buffers.cpp
    ${COMMON_SRC_DIR}/transport/transmit_buffer_pool.h
    ${COMMON_SRC_DIR}/transport/transmit_buffer_pool.cpp
    ${COMMON_SRC_DIR}/transport/transport.h
    ${COMMON_SRC_DIR}/transport/transport.cpp
    ${COMMON_SRC_DIR}/transport/transport_reader.cpp

    # ${COMMON_SRC_DIR}/pid.h
    # ${COMMON_SRC_DIR}/pid.cpp
)
target_include_directories(test_command PRIVATE ${COMMON_SRC_DIR})
target_compile_definitions(test_command PUBLIC TRANSPORT_MODE_LINE=1)
pico_enable_stdio_usb(test_command 1)
pico_enable_stdio_uart(test_command 0)
pico_add_extra_outputs(test_command)
target_link_libraries(test_command pico_stdlib hardware_gpio hardware_pwm)

add_executable(test_02_command 
    test_02_command.cpp
    ${COMMON_SRC_DIR}/cli/argv.cpp
    ${COMMON_SRC_DIR}/cli/argv.h
    # ${COMMON_SRC_DIR}/cli/commands.h
    # ${COMMON_SRC_DIR}/cli/commands.cpp

    # ${COMMON_SRC_DIR}/enum.cpp
    # ${COMMON_SRC_DIR}/enum.h
    ${COMMON_SRC_DIR}/trace.cpp
    ${COMMON_SRC_DIR}/trace.h
    # ${COMMON_SRC_DIR}/dri0002.h
    # ${COMMON_SRC_DIR}/dri0002.cpp
    # ${COMMON_SRC_DIR}/encoder.h
    # ${COMMON_SRC_DIR}/encoder.cpp
    # ${COMMON_SRC_DIR}/encoder_sample.h
    # ${COMMON_SRC_DIR}/encoder_sample.cpp
    # ${COMMON_SRC_DIR}/pico_gpio_irq_dispatcher.h
    # ${COMMON_SRC_DIR}/pico_gpio_irq_dispatcher.cpp
    # ${COMMON_SRC_DIR}/motion.h
    # ${COMMON_SRC_DIR}/motion.cpp
    # ${COMMON_SRC_DIR}/robot.h
    # ${COMMON_SRC_DIR}/robot.cpp
    # ${COMMON_SRC_DIR}/task.h

    # ${COMMON_SRC_DIR}/transport/buffers.h
    ${COMMON_SRC_DIR}/transport/buffers.cpp
    ${COMMON_SRC_DIR}/transport/transmit_buffer_pool.h
    ${COMMON_SRC_DIR}/transport/transmit_buffer_pool.cpp
    ${COMMON_SRC_DIR}/transport/transport.h
    ${COMMON_SRC_DIR}/transport/transport.cpp
    ${COMMON_SRC_DIR}/transport/transport_reader.cpp

    # ${COMMON_SRC_DIR}/pid.h
    # ${COMMON_SRC_DIR}/pid.cpp
)
target_include_directories(test_02_command PRIVATE ${COMMON_SRC_DIR})
target_compile_definitions(test_02_command PUBLIC TRANSPORT_MODE_LINE=1)
pico_enable_stdio_usb(test_02_command 1)
pico_enable_stdio_uart(test_02_command 0)
pico_add_extra_outputs(test_02_command)
target_link_libraries(test_02_command pico_stdlib hardware_gpio hardware_pwm)