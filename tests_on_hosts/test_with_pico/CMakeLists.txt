
set(TARGET test_with_pico)
add_executable(${TARGET}
		test_with_pico.cpp
		${FIRMWARE_COMMON_DIR}/closed_loop/custom.h
		${FIRMWARE_COMMON_DIR}/closed_loop/custom.cpp
		${FIRMWARE_COMMON_DIR}/closed_loop/value_with_direction.h
		${FIRMWARE_COMMON_DIR}/closed_loop/motor_profile.h
		${FIRMWARE_COMMON_DIR}/closed_loop/motor_profile.cpp
		${TESTS_COMMON_DIR}/unittest.h
		${TESTS_COMMON_DIR}/unittest.c
		${BRIDGE_LIB_DIR}/serial_settings.h
		${BRIDGE_LIB_DIR}/serial_settings.cpp
		${FIRMWARE_COMMON_DIR}/trace.h
		${FIRMWARE_COMMON_DIR}/trace.cpp
)
target_compile_definitions(${TARGET}
		PUBLIC
		FIRMWARE_SRC_COMMON_DIR_JSON_OUTPUT_PATH="${CMAKE_CURRENT_LIST_DIR}/json.txt"
)
target_include_directories(${TARGET}
		PUBLIC
		${JSON_LIB_DIR}
		${CPP_PKG_DIR}/src
		${FIRMWARE_COMMON_DIR}
		${TESTS_COMMON_DIR}
		${JSONCONS_DIR}
)
target_link_libraries(${TARGET} /usr/lib/x86_64-linux-gnu/libserial.so)

add_test(${TARGET} ${TARGET})

