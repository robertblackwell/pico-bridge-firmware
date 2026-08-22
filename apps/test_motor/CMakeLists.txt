message("COMMON_SRC_DIR    ${COMMON_SRC_DIR}")

add_executable(motor_test 
    motor_test.cpp
)
target_link_libraries(motor_test PRIVATE common_logic pico_stdlib hardware_gpio hardware_pwm)