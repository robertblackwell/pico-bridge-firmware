
add_executable(closed_loop 
    closed_loop.cpp
    ${COMMON_SOURCE_FILES}
)
target_include_directories(closed_loop PRIVATE 
    ${COMMON_SRC_DIR}
    ${PICO_FIRMWARE_DIR}
    ${PROJECT_BINARY_DIR}
)
target_compile_definitions(closed_loop 
    PUBLIC 
    TRANSPORT_MODE_LINE=1
    # ENCODER_ISR_ALGORITHM_V1=2
)
pico_enable_stdio_usb(closed_loop 1)
pico_enable_stdio_uart(closed_loop 0)
pico_add_extra_outputs(closed_loop)
target_link_libraries(closed_loop pico_stdlib hardware_gpio hardware_pwm hardware_sync)