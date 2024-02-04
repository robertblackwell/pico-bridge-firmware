
MESSAGE("Entered test_serial_ports COMMON_SRC_DIR ${COMMON_SRC_DIR}")

# stdio via usb
set(TargetStdioUsb test_stdio_usb)
add_executable(${TargetStdioUsb} 
    test_stdio_usb.cpp
    )
pico_enable_stdio_usb(${TargetStdioUsb} 1)
pico_enable_stdio_uart(${TargetStdioUsb} 0)
pico_add_extra_outputs(${TargetStdioUsb})
target_include_directories(${TargetStdioUsb} PUBLIC ${COMMON_SRC_DIR})
target_link_libraries(${TargetStdioUsb} pico_stdlib pico_unique_id)

# stdio via UART0
set(TargetStdioUart0 test_stdio_uart0)
add_executable(${TargetStdioUart0} 
    test_stdio_uart0.cpp
    )
pico_enable_stdio_usb(${TargetStdioUart0} 0)
pico_enable_stdio_uart(${TargetStdioUart0} 1)
pico_add_extra_outputs(${TargetStdioUart0})
target_include_directories(${TargetStdioUart0} PUBLIC ${COMMON_SRC_DIR})
target_link_libraries(${TargetStdioUart0} pico_stdlib pico_unique_id)
target_compile_definitions(${TargetStdioUart0} PRIVATE
    PICO_DEFAULT_UART_RX_PIN=17
    PICO_DEFAULT_UART_TX_PIN=16
)

# test uart1
set(TargetUart1 test_uart1)
add_executable(${TargetUart1} 
    test_uart1.cpp)

pico_enable_stdio_usb(${TargetUart1} 1)
pico_enable_stdio_uart(${TargetUart1} 0)
pico_add_extra_outputs(${TargetUart1})
target_include_directories(${TargetUart1} PUBLIC ${COMMON_SRC_DIR})
target_link_libraries(${TargetUart1} pico_stdlib pico_stdio_uart pico_unique_id)
# target_compile_definitions(${TARGET} PRIVATE
#     PICO_DEFAULT_UART_RX_PIN=8
#     PICO_DEFAULT_UART_TX_PIN=9
# )


# test stdio = uart0 and uart1 at the same time
set(TargetUart0Uart1 test_uart0_uart1)
add_executable(${TargetUart0Uart1} 
    test_uart0_uart1.cpp)

pico_enable_stdio_usb(${TargetUart0Uart1} 0)
pico_enable_stdio_uart(${TargetUart0Uart1} 1)

pico_add_extra_outputs(${TargetUart0Uart1})
target_include_directories(${TargetUart0Uart1} PUBLIC ${COMMON_SRC_DIR})
target_link_libraries(${TargetUart0Uart1} pico_stdlib pico_unique_id)
target_compile_definitions(${TargetUart0Uart1} PRIVATE
    PICO_DEFAULT_UART_RX_PIN=17
    PICO_DEFAULT_UART_TX_PIN=16
)

# test stdio = uart0 and uart1 at the same time
set(TargetUart0 test_uart0)
add_executable(${TargetUart0} 
    test_uart0.cpp)

pico_enable_stdio_usb(${TargetUart0} 1)
pico_enable_stdio_uart(${TargetUart0} 0)

pico_add_extra_outputs(${TargetUart0})
target_include_directories(${TargetUart0} PUBLIC ${COMMON_SRC_DIR})
target_link_libraries(${TargetUart0} pico_stdlib pico_unique_id)
# target_compile_definitions(${TargetUart0} PRIVATE
#     PICO_DEFAULT_UART_RX_PIN=17
#     PICO_DEFAULT_UART_TX_PIN=16
# )
