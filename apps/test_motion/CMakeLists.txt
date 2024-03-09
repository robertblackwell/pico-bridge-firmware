message("COMMON_SRC_DIR    ${COMMON_SRC_DIR}")

add_executable(motion_test 
    motion_test.cpp
    ${COMMON_SRC_DIR}/trace.cpp
    ${COMMON_SRC_DIR}/trace.h
    ${COMMON_SRC_DIR}/dri0002.h
    ${COMMON_SRC_DIR}/dri0002.cpp
    ${COMMON_SRC_DIR}/encoder.h
    ${COMMON_SRC_DIR}/encoder.cpp
    ${COMMON_SRC_DIR}/pico_gpio_irq_dispatcher.h
    ${COMMON_SRC_DIR}/pico_gpio_irq_dispatcher.cpp
    ${COMMON_SRC_DIR}/motion.h
    ${COMMON_SRC_DIR}/motion.cpp
    ${COMMON_SRC_DIR}/reporter.h
    ${COMMON_SRC_DIR}/reporter.cpp
    exercise_motors.h
    exercise_motors.cpp
    ${COMMON_SRC_DIR}/transport.h
    ${COMMON_SRC_DIR}/transport.cpp

    )
target_include_directories(motion_test PRIVATE ${COMMON_SRC_DIR})
pico_enable_stdio_usb(motion_test 1)
pico_enable_stdio_uart(motion_test 0)
pico_add_extra_outputs(motion_test)
target_link_libraries(motion_test pico_stdlib hardware_gpio hardware_pwm)