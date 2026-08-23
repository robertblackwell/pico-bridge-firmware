
set(TARGET test_main_loop)
add_executable(${TARGET} 
    test_main.cpp
    ${TESTS_COMMON_DIR}/unittest.c
    ${TESTS_COMMON_DIR}/unittest.h
)

target_link_libraries(${TARGET} PRIVATE common_logic Threads::Threads )

target_compile_definitions(${TARGET} 
    PUBLIC 
        LOCAL_TEST=1 
        TRANSPORT_MODE_PACKET=0
)

target_include_directories(${TARGET} 
    PUBLIC 
        ${TESTS_COMMON_DIR}
)

add_test(${TARGET} ${TARGET})

