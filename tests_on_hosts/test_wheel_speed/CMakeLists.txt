set(TARGET test_wheel_speed)
add_executable(${TARGET}
    test_wheel_speed.cpp
    ${TESTS_COMMON_DIR}/unittest.c
    ${TESTS_COMMON_DIR}/unittest.h
)

target_link_libraries(${TARGET} PRIVATE common_logic)

target_compile_options(${TARGET} PRIVATE -Wformat -pthread)

target_compile_definitions(${TARGET}
    PUBLIC
        LOCAL_TEST=1
)

target_include_directories(${TARGET}
    PUBLIC
        ../common
        ${FIRMWARE_COMMON_DIR}
)

target_link_options(${TARGET} PRIVATE -pthread)

add_test(${TARGET} ${TARGET})
