message("COMMON_SRC_DIR    ${COMMON_SRC_DIR}")

add_executable(micros_timing
    micros_timing.cpp
)
target_include_directories(micros_timing PRIVATE ${COMMON_SRC_DIR})
target_compile_definitions(micros_timing
    PUBLIC 
    TRANSPORT_MODE_LINE=1
    # ENCODER_ISR_ALGORITHM_V1=2
)
pico_enable_stdio_usb(micros_timing 1)
pico_enable_stdio_uart(micros_timing 0)
pico_add_extra_outputs(micros_timing)
target_link_libraries(micros_timing pico_stdlib hardware_gpio hardware_pwm hardware_sync)