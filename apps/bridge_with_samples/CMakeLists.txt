add_executable(bridge_with_samples 
    bridge_with_samples.cpp
)
target_include_directories(bridge_with_samples PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${COMMON_SRC_DIR}
    ${PICO_FIRMWARE_DIR}
    ${PROJECT_BINARY_DIR}
)
target_compile_definitions(bridge_with_samples 
    PUBLIC 
    TRANSPORT_MODE_LINE=1
    # ENCODER_ISR_ALGORITHM_V1=2
)
pico_enable_stdio_usb(bridge_with_samples 1)
pico_enable_stdio_uart(bridge_with_samples 0)
pico_add_extra_outputs(bridge_with_samples)
target_link_libraries(bridge_with_samples PRIVATE common_logic pico_stdlib hardware_sync)