add_executable(bridge 
    bridge.cpp
)
target_include_directories(bridge PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${COMMON_SRC_DIR}
    ${PICO_FIRMWARE_DIR}
    ${PROJECT_BINARY_DIR}
)
target_compile_definitions(bridge 
    PUBLIC 
    TRANSPORT_MODE_LINE=1
    # ENCODER_ISR_ALGORITHM_V1=2
)
pico_enable_stdio_usb(bridge 1)
pico_enable_stdio_uart(bridge 0)
pico_add_extra_outputs(bridge)
target_link_libraries(bridge PRIVATE common_logic pico_stdlib hardware_sync)