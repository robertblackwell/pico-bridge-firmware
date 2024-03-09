message("COMMON_SRC_DIR    ${COMMON_SRC_DIR}")

add_executable(motor_test 
    motor_test.cpp
    ${COMMON_SRC_DIR}/trace.cpp
    ${COMMON_SRC_DIR}/trace.h
    ${COMMON_SRC_DIR}/enum.h
    ${COMMON_SRC_DIR}/enum.cpp
    ${COMMON_SRC_DIR}/dri0002.h
    ${COMMON_SRC_DIR}/dri0002.cpp
    )
target_include_directories(motor_test PRIVATE ${COMMON_SRC_DIR})
pico_enable_stdio_usb(motor_test 1)
pico_enable_stdio_uart(motor_test 0)
pico_add_extra_outputs(motor_test)
target_link_libraries(motor_test pico_stdlib hardware_gpio hardware_pwm)