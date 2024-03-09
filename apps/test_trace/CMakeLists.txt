
MESSAGE("Entered tests/test_trace  PICO_SRC_DIR    ${PICO_SRC_DIR}")

# stdio via usb
set(TargetStdio test_stdio_usb_trace)
add_executable(${TargetStdio} 
    test_stdio_trace.cpp
    ${PICO_SRC_DIR}/common/trace.h
    ${PICO_SRC_DIR}/common/trace.cpp
    )
pico_enable_stdio_usb(${TargetStdio} 1)
pico_enable_stdio_uart(${TargetStdio} 0)
pico_add_extra_outputs(${TargetStdio})
target_include_directories(${TargetStdio} PUBLIC ${PICO_SRC_DIR} ${PICO_SRC_DIR}/common)
target_link_libraries(${TargetStdio} pico_stdlib pico_unique_id)

# stdio via UART0
set(TargetStdio test_stdio_uart0_trace)
add_executable(${TargetStdio} 
    test_stdio_trace.cpp
    ${PICO_SRC_DIR}/common/trace.h
    ${PICO_SRC_DIR}/common/trace.cpp
    )
pico_enable_stdio_usb(${TargetStdio} 0)
pico_enable_stdio_uart(${TargetStdio} 1)
pico_add_extra_outputs(${TargetStdio})
target_include_directories(${TargetStdio} PUBLIC ${PICO_SRC_DIR} ${PICO_SRC_DIR}/common)
target_link_libraries(${TargetStdio} pico_stdlib pico_unique_id)


# uart0 and uart1
set(TargetUart test_uart_trace)
add_executable(${TargetUart} 
    test_uart_trace.cpp
    ${PICO_SRC_DIR}/common/trace.h 
    ${PICO_SRC_DIR}/common/trace.cpp)
pico_enable_stdio_usb(${TargetUart} 1)
pico_enable_stdio_uart(${TargetUart} 0)
pico_add_extra_outputs(${TargetUart})
target_include_directories(${TargetUart} PUBLIC ${PICO_SRC_DIR} ${PICO_SRC_DIR}/common)
target_link_libraries(${TargetUart} pico_stdlib pico_unique_id)