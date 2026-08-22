
set(TARGET test_static_buffers_cpp)
add_executable(${TARGET}
    test_static_buffers_cpp.cpp
    ${TESTS_COMMON_DIR}/unittest.c
    ${TESTS_COMMON_DIR}/unittest.h
)
target_link_libraries(${TARGET} PRIVATE common_logic)

target_compile_options(${TARGET} PRIVATE -Wformat) 

target_compile_definitions(${TARGET} 
	PUBLIC 
		LOCAL_TEST=1 
		TRANSPORT_MODE_PACKET=1
)
target_include_directories(${TARGET} 
	PUBLIC 
		../common 
		${FIRMWARE_COMMON_DIR})

add_test(${TARGET} ${TARGET})

