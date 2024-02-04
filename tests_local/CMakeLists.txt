cmake_minimum_required(VERSION 3.16)

project(robot_linux_tests VERSION 0.1.0 LANGUAGES CXX C)
find_package(Threads REQUIRED)
find_program(BASH_PROG bash)
message("BASH_PROG-NOTFOUND       ${BASH_PROG-NOTFOUND}")
message("BASH_PROG                ${BASH_PROG}")

## not sure any of these work for the moment so turn them OFF
if(OFF)
# # find_package(boost)
# # find_package(openssl)
# find_package(trog)
# find_package(x509_certificate_library)
endif()

##
## options
##
option(LOCAL_TESTS_BuildTests "Build the tests when enabled" ON )
option(LOCAL_TESTS_Install_Targets "Installs targets as well as files if set" OFF)
option(LOCAL_TESTS_Package "Install as a CMake package - not implemented" OFF)
option(LOCAL_TESTS_Verbose "Print a lot of diagnostic stuff" ON)
option(LOCAL_TESTS_DebugBuild "Perform build as type Debug" ON)
option(LOCAL_TESTS_BuildDocs "Build doxygen documentation")

## ============================================================================
## The stuff below - I am not sure about ======================================
##

# this is a fix for a problem with boost libraries see https://github.com/Microsoft/vcpkg/issues/4497
# also a hack and breaks "best practices" as per Damiel Pfeiffer's presentation https://www.youtube.com/watch?v=rLopVhns4Zs&feature=youtu.be
set(other_flags "${other_flags} -frtti -fvisibility-inlines-hidden")
set(other_flags "${other_flags} -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${other_flags} -pthread")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
set(CTEST_CUSTOM_PRE_TEST "echo THIS IS PRE TEST")
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Let's nicely support folders in IDE's
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# allow scheme porperties in xcode - particularly environment variables
set_property(GLOBAL PROPERTY XCODE_GENERATE_SCHEME ON)
set(XCODE_GENERATE_SCHEME ON)
##
## The stuff below - I am not sure about ====================================
## ==========================================================================

##
##
## Configuration
##
include (GNUInstallDirs)

if (LOCAL_TESTS_DebugBuild)
	set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)
if(DEFINED CMAKE_DL_LIBS)
    set(LOCAL_TESTS_DL_LIB_SO libdl.so)
endif()

set(LOCAL_TESTS_PROJECT_DIR     ${LOCAL_TESTS_SOURCE_DIR})
## location of project header and source files
set(LOCAL_TESTS_SOURCE_NAME      LOCAL_TESTS)
set(LOCAL_TESTS_INCLUDE_NAME     LOCAL_TESTS)
set(LOCAL_TESTS_SOURCE_DIR       ${LOCAL_TESTS_PROJECT_DIR}/${LOCAL_TESTS_SOURCE_NAME})
set(LOCAL_TESTS_INCLUDE_DIR      ${LOCAL_TESTS_PROJECT_DIR}/${LOCAL_TESTS_INCLUDE_NAME})
## location of other/3rd party headers, src and libs
set(LOCAL_TESTS_VENDOR_NAME		vendor)
set(LOCAL_TESTS_VENDOR_DIR       ${LOCAL_TESTS_PROJECT_DIR}/${LOCAL_TESTS_VENDOR_NAME})
set(LOCAL_TESTS_VENDOR_INCLUDEDIR ${LOCAL_TESTS_VENDOR_DIR}/include)
set(LOCAL_TESTS_VENDOR_LIBDIR    ${LOCAL_TESTS_VENDOR_DIR}/lib)
set(LOCAL_TESTS_VENDOR_SRCDIR    ${LOCAL_TESTS_VENDOR_DIR}/src)

if (${LOCAL_TESTS_BuildDocs} AND (DEFINED DOXYGEN_FOUND))
	set(LOCAL_TESTS_BUILD_DOCS TRUE)
else()
	set(LOCAL_TESTS_BUILD_DOCS FALSE)
endif()

## used by all compile steps to find headers
set(LOCAL_TESTS_INCLUDE_PATHS 
	${LOCAL_TESTS_PROJECT_DIR}
	# ${LOCAL_TESTS_INCLUDE_DIR}
	# ${LOCAL_TESTS_SOURCE_DIR}
	${LOCAL_TESTS_VENDOR_INCLUDEDIR}
	${LOCAL_TESTS_VENDOR_SRCDIR}
)
set(LOCAL_TESTS_LINK_LIBRARIES 
	LOCAL_TESTS_library 
	Threads::Threads 
	${CMAKE_DL_LIBS}
	${LOCAL_TESTS_VENDOR_LIBDIR}/libcert_library.a
	${LOCAL_TESTS_VENDOR_LIBDIR}/libboost_filesystem.a 
	${LOCAL_TESTS_VENDOR_LIBDIR}/libboost_system.a
	${LOCAL_TESTS_VENDOR_LIBDIR}/libboost_program_options.a
	${LOCAL_TESTS_VENDOR_LIBDIR}/libssl.a 
	${LOCAL_TESTS_VENDOR_LIBDIR}/libcrypto.a 
	${CMAKE_DL_LIBS}
	${DL_LIB_SO}
	)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY XCODE_GENERATE_SCHEME ON)

if (LOCAL_TESTS_Verbose)
	message("Project Name                      ${CMAKE_PROJECT_NAME}")
	message("Build Type                        ${CMAKE_BUILD_TYPE}")
	message("CMAKE_CXX_STANDARD                ${CMAKE_CXX_STANDARD}")
	message("CMAKE_DL_LIBS                     ${CMAKE_DL_LIBS}")
	message("LOCAL_TESTS_PROJECT_DIR                ${LOCAL_TESTS_PROJECT_DIR}")
	message("LOCAL_TESTS_SOURCE_DIR                 ${LOCAL_TESTS_SOURCE_DIR}")
	message("LOCAL_TESTS_INCLUDE_DIR                ${LOCAL_TESTS_INCLUDE_DIR}")
	message("LOCAL_TESTS_VENDOR_NAME                ${LOCAL_TESTS_VENDOR_NAME}")
	message("LOCAL_TESTS_VENOR_DIR                  ${LOCAL_TESTS_VENDOR_DIR}")
	message("LOCAL_TESTS_VENDOR_INCLUDEDIR          ${LOCAL_TESTS_VENDOR_INCLUDEDIR}")
	message("LOCAL_TESTS_VENDOR_LIBDIR              ${LOCAL_TESTS_VENDOR_LIBDIR}")
	message("LOCAL_TESTS_VENDOR_SRCDIR              ${LOCAL_TESTS_VENDOR_SRCDIR}")
	message("LOCAL_TESTS_VENDOR_SRCDIR              ${LOCAL_TESTS_VENDOR_SRCDIR}")
	message("LOCAL_TESTS_DL_LIB_SO                  ${LOCAL_TESTS_DL_LIB_SO}")
	message("LOCAL_TESTS_INCLUDE_PATHS              ${LOCAL_TESTS_INCLUDE_PATHS}")
	message("LOCAL_TESTS_LINK_LIBRARIES             ${LOCAL_TESTS_LINK_LIBRARIES}")
	message("LOCAL_TESTS_BuildDocsS                 ${LOCAL_TESTS_BuildDocs}")
	message("DOXYGEN_FOUND                     ${DOXYGEN_FOUND}")
	message("LOCAL_TESTS_BUILD_DOCS                 ${LOCAL_TESTS_BUILD_DOCS}")
	message("CMAKE_INSTALL_PREFIX              ${CMAKE_INSTALL_PREFIX}")
	message("CMAKE_INSTALL_FULL_INCLUDEDIR     ${CMAKE_INSTALL_FULL_INCLUDEDIR}")
	message("CMAKE_INSTALL_INCLUDEDIR          ${CMAKE_INSTALL_INCLUDEDIR}")

endif()

##
## Enabled testing
##
include(CTest) 
enable_testing()

add_subdirectory(test_main_loop)

# add_subdirectory(test_static_buffer_cpp)

