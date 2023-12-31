
message("COMMON   ${COMMON_SRC_DIR}")
set(TargetName proof_of_concept)
add_executable(${TargetName} 
    proof_of_concept.cpp
    )
target_include_directories(${TargetName} PRIVATE ${COMMON_SRC_DIR})
pico_enable_stdio_usb(${TargetName} 0)
pico_enable_stdio_uart(${TargetName} 1)
target_compile_definitions(${TargetName} PRIVATE
    # PICO_DEFAULT_UART_RX_PIN=16
    # PICO_DEFAULT_UART_TX_PIN=17
    PICO_DEFAULT_UART=1
    PICO_DEFAULT_UART_RX_PIN=9
    PICO_DEFAULT_UART_TX_PIN=8
)




pico_add_extra_outputs(${TargetName})
target_link_libraries(${TargetName} pico_stdlib)


add_executable(clitest 
    clitest.cpp
    ${COMMON_SRC_DIR}/cli.cpp
    ${COMMON_SRC_DIR}/cli.h
    ${COMMON_SRC_DIR}/utils.cpp
    ${COMMON_SRC_DIR}/utils.h
    )
target_include_directories(clitest PRIVATE ${COMMON_SRC_DIR})
pico_enable_stdio_usb(clitest 1)
pico_enable_stdio_uart(clitest 0)
pico_add_extra_outputs(clitest)
target_link_libraries(clitest pico_stdlib)

# 
# Command test
# 
set(TargetName command_test)
add_executable(${TargetName} 
    command_test.cpp
    ${COMMON_SRC_DIR}/cli.cpp
    ${COMMON_SRC_DIR}/cli.h
    ${COMMON_SRC_DIR}/utils.cpp
    ${COMMON_SRC_DIR}/utils.h
    ${COMMON_SRC_DIR}/commands.cpp
    ${COMMON_SRC_DIR}/commands.h
    )
target_include_directories(${TargetName} PRIVATE ${COMMON_SRC_DIR})
# pico_enable_stdio_usb(${TargetName} 1)
# pico_enable_stdio_uart(${TargetName} 0)

pico_enable_stdio_usb(${TargetName} 0)
pico_enable_stdio_uart(${TargetName} 1)
pico_add_extra_outputs(${TargetName})
target_compile_definitions(${TargetName} PRIVATE
    # PICO_DEFAULT_UART_RX_PIN=16
    # PICO_DEFAULT_UART_TX_PIN=17
    PICO_DEFAULT_UART=1
    PICO_DEFAULT_UART_RX_PIN=9
    PICO_DEFAULT_UART_TX_PIN=8
    )


pico_add_extra_outputs(command_test)
target_link_libraries(command_test pico_stdlib)

