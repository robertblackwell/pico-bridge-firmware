
set(TARGET test_static_buffers_cpp)
add_executable(${TARGET}
		test_static_buffers_cpp.cpp
	${TESTS_COMMON_DIR}/unittest.c
	${TESTS_COMMON_DIR}/unittest.h
	${FIRMWARE_COMMON_DIR}/trace.cpp
	${FIRMWARE_COMMON_DIR}/trace.h
	${FIRMWARE_COMMON_DIR}/encoder_sample.h
	${FIRMWARE_COMMON_DIR}/encoder_sample.cpp
	${FIRMWARE_COMMON_DIR}/cli/argv.h
	${FIRMWARE_COMMON_DIR}/cli/argv.cpp
	${FIRMWARE_COMMON_DIR}/cli/commands.h
	${FIRMWARE_COMMON_DIR}/cli/commands.cpp
	${FIRMWARE_COMMON_DIR}/transport/transport.h
	${FIRMWARE_COMMON_DIR}/transport/transport.cpp
	${FIRMWARE_COMMON_DIR}/transport/buffers.cpp
	${FIRMWARE_COMMON_DIR}/transport/buffers.h
	${FIRMWARE_COMMON_DIR}/transport/transmit_buffer_pool.cpp
	${FIRMWARE_COMMON_DIR}/transport/transmit_buffer_pool.h
	# ${FIRMWARE_COMMON_DIR}/transport/transport_reader.cpp
)
target_compile_options(${TARGET} PRIVATE -Wformat) 

target_compile_definitions(${TARGET} 
	PUBLIC 
		LOCAL_TEST=1 
		PLATFORM_IS_LINUX=1, 
		TRANSPORT_MODE_PACKET=1
)
target_include_directories(${TARGET} 
	PUBLIC 
		../common 
		${FIRMWARE_COMMON_DIR}
		${LOCAL_TESTS_INCLUDE_PATHS})

add_test(${TARGET} ${TARGET})

