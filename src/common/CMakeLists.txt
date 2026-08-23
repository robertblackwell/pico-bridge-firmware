# This file defines the common logic libraries for both Pico and Host builds.

# 1. Truly Agnostic Logic (No hardware dependencies)
set(AGNOSTIC_SOURCES
    cli/argv.cpp
    cli/commands.cpp
    cli/execute_commands.cpp
    enum.cpp
    trace.cpp
    encoder_sample.cpp
    encoder_debug.cpp
    pid.cpp
    transport/buffers.cpp
    transport/transmit_buffer_pool.cpp
    transport/transport.cpp
    transport/transport_reader.cpp
        hw_utils.h
)

add_library(common_agnostic ${AGNOSTIC_SOURCES})
target_include_directories(common_agnostic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT PICO_ON_DEVICE)
    target_compile_definitions(common_agnostic PUBLIC PLATFORM_IS_LINUX=1)
endif()

# 2. Firmware Logic (May have hardware dependencies)
set(FIRMWARE_SOURCES
    motion.cpp
    reporter.cpp
    robot.cpp
    dri0002.cpp
    encoder_v2.cpp
    pico_gpio_irq_dispatcher.cpp
)

if(PICO_ON_DEVICE)
    add_library(common_firmware ${FIRMWARE_SOURCES})
    target_link_libraries(common_firmware PUBLIC common_agnostic pico_stdlib hardware_gpio hardware_pwm hardware_irq hardware_sync)
    
    # Also link agnostic logic to pico_stdlib when on device
    target_link_libraries(common_agnostic PUBLIC pico_stdlib)

    # Combined library for easier linking in Pico apps
    add_library(common_logic INTERFACE)
    target_link_libraries(common_logic INTERFACE common_firmware)
else()
    # On host, we might still want to try building firmware logic if mocks are provided
    # For now, we only provide agnostic logic to the tests that need it.
    add_library(common_logic INTERFACE)
    target_link_libraries(common_logic INTERFACE common_agnostic)
endif()
