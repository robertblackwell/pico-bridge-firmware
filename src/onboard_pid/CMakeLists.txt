cmake_minimum_required(VERSION 3.13)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(TargetName onboard_pid)
include(pico_sdk_import.cmake)
project(${TargetName})
pico_sdk_init()
add_compile_options(-DMYPROG_NAME=${TargetName})

set(COMMON_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/../common)
message("COMMON_SRC_DIR    ${COMMON_SRC_DIR}")

add_executable(${TargetName} 
    main.cpp
    ${COMMON_SRC_DIR}/enum.cpp
    ${COMMON_SRC_DIR}/enum.h
    ${COMMON_SRC_DIR}/trace.cpp
    ${COMMON_SRC_DIR}/trace.h
    ${COMMON_SRC_DIR}/dri0002.h
    ${COMMON_SRC_DIR}/dri0002.cpp
    ${COMMON_SRC_DIR}/encoder.h
    ${COMMON_SRC_DIR}/encoder.cpp
    ${COMMON_SRC_DIR}/pico_gpio_irq_dispatcher.h
    ${COMMON_SRC_DIR}/pico_gpio_irq_dispatcher.cpp
    ${COMMON_SRC_DIR}/cli.h
    ${COMMON_SRC_DIR}/cli.cpp
    ${COMMON_SRC_DIR}/commands.h
    ${COMMON_SRC_DIR}/commands.cpp
    ${COMMON_SRC_DIR}/motion.h
    ${COMMON_SRC_DIR}/motion.cpp
    ${COMMON_SRC_DIR}/reporter.h
    ${COMMON_SRC_DIR}/reporter.cpp
    ${COMMON_SRC_DIR}/robot.h
    ${COMMON_SRC_DIR}/robot.cpp

    ${COMMON_SRC_DIR}/task.h
    ${COMMON_SRC_DIR}/transport.h
    ${COMMON_SRC_DIR}/transport.cpp
    ${COMMON_SRC_DIR}/pid.h
    ${COMMON_SRC_DIR}/pid.cpp
)
target_include_directories(${TargetName} PRIVATE ${COMMON_SRC_DIR})
pico_enable_stdio_usb(${TargetName} 1)
pico_enable_stdio_uart(${TargetName} 0)
pico_add_extra_outputs(${TargetName})
target_compile_definitions(${TargetName} PRIVATE
    PICO_DEFAULT_UART_RX_PIN=16
    PICO_DEFAULT_UART_TX_PIN=17
)
target_link_libraries(${TargetName} pico_stdlib hardware_gpio hardware_pwm)