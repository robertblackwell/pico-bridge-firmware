
set(TARGET test_static_buffers_cpp)
add_executable(${TARGET}
		test_static_buffers_cpp.cpp
	../common/unittest.c
	../common/unittest.h
	../../src/common/static_buffers.cpp
	../../src/common/static_buffers.h
	../../src/common/transmit_buffer_pool.cpp
	../../src/common/transmit_buffer_pool.h
	../../src/common/trace.cpp
	../../src/common/trace.h
	../../src/common/encoder_sample.h
	../../src/common/encoder_sample.cpp
	../../src/common/argv.h
	../../src/common/argv.cpp
	../../src/common/commands.h
	../../src/common/commands.cpp
	../../src/common/transport.h
	../../src/common/transport.cpp
	# ../../src/common/transport_reader.cpp
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
		../../src/common
		${LOCAL_TESTS_INCLUDE_PATHS})

add_test(${TARGET} ${TARGET})

