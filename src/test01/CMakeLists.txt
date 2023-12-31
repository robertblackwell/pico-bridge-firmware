# cmake_minimum_required(VERSION 3.13)
# include(pico_sdk_import.cmake)
# project(test_project)
# pico_sdk_init()

set(TargetName test01)

add_executable(${TargetName} test.cpp)

pico_enable_stdio_usb(${TargetName} 0)
pico_enable_stdio_uart(${TargetName} 1)
pico_add_extra_outputs(${TargetName})

target_compile_definitions(${TargetName} PRIVATE
    # PICO_DEFAULT_UART_RX_PIN=16
    # PICO_DEFAULT_UART_TX_PIN=17
    PICO_DEFAULT_UART=1
    PICO_DEFAULT_UART_RX_PIN=9
    PICO_DEFAULT_UART_TX_PIN=8

)

target_link_libraries(${TargetName} pico_stdlib pico_unique_id)