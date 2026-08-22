# cmake_minimum_required(VERSION 3.13)
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_C_STANDARD 11)
# set(CMAKE_CXX_STANDARD 17)
# include(pico_sdk_import.cmake)
# project(test_project)
# pico_sdk_init()
# set(COMMON_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/../common)
message("COMMON_SRC_DIR    ${COMMON_SRC_DIR}")

add_executable(test_command 
    test_command.cpp
)
target_link_libraries(test_command PRIVATE common_logic pico_stdlib hardware_gpio hardware_pwm)

add_executable(test_02_command 
    test_02_command.cpp
)
target_link_libraries(test_02_command PRIVATE common_logic pico_stdlib hardware_gpio hardware_pwm)