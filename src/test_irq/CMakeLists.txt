# cmake set(TARGET test_irq)
add_executable(test_irq 
    test_irq.cpp
    ${COMMON_SRC_DIR}/utils.cpp
    ${COMMON_SRC_DIR}/utils.h
    ${COMMON_SRC_DIR}/pico_gpio_irq_dispatcher.h
    ${COMMON_SRC_DIR}/pico_gpio_irq_dispatcher.cpp
)
pico_enable_stdio_usb(test_irq 1)
pico_enable_stdio_uart(test_irq 0)
pico_add_extra_outputs(test_irq)
target_include_directories(test_irq PRIVATE ${COMMON_SRC_DIR})
target_link_libraries(test_irq pico_stdlib hardware_gpio)