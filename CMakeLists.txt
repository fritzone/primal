cmake_minimum_required (VERSION 2.8.11)

project(primal)
include (TestBigEndian)

if(NOT ${TEST_ENDIAN} EQUAL 1)
    message("Compiling on a little nedian machine")
    add_definitions( -DCOMPILED_ON_LITTLE_ENDIAN )
endif()

option(coverage-build "gcov/lcov test coverage analysis (make coverage_test)" OFF)

set( CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT WIN32)
    add_definitions(-Wall -Wextra -Wcomment -Wformat -Winit-self -Wno-long-long -Wmissing-include-dirs -Wno-unused-parameter -Wno-unused-variable -std=c++14)
    option(gprof-build "enable gprof" OFF)
    if(gprof-build)
        add_definitions("-pg")
    endif()
else()
    add_definitions(/W4)
    add_definitions(/D_CRT_SECURE_NO_WARNINGS)
    add_definitions(/D_SCL_SECURE_NO_WARNINGS)
endif()

# To specify the word size of the VM: 32 bit or 64 bit
set(TARGET_ARCH "32" CACHE STRING "The selected architecture's size")
set_property(CACHE TARGET_ARCH PROPERTY STRINGS "32" "64")

message(Building: ${TARGET_ARCH} bit machine)

if(${TARGET_ARCH} EQUAL 32)
    add_definitions("-DTARGET_ARCH=32")
elseif(${TARGET_ARCH} EQUAL 64)
    add_definitions("-DTARGET_ARCH=64")
endif()

# These will go into all submodules for now
include_directories(${ROOT_DIR}/opcodes)
include_directories(${ROOT_DIR}/hal)
include_directories(${ROOT_DIR}/compiler)
include_directories(${ROOT_DIR}/vm)
include_directories(${ROOT_DIR}/util)

add_subdirectory(util)

# Firstly this, to create the opcode headers
add_subdirectory(opcodes)
add_subdirectory(hal)

# Need this to have the opcode headers in the include path
include_directories(${CMAKE_BINARY_DIR}/opcodes)

if(coverage-build)
    set(CMAKE_BUILD_TYPE Debug)
    message("building for coverage ...")
    SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage -rdynamic")
    SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage -rdynamic")

    include(cmake/CodeCoverage.cmake)
    SETUP_TARGET_FOR_COVERAGE(coverage_test ctest coverage)
endif()

if(gprof-build)
    set(CMAKE_EXE_LINKER_FLAGS "-pg")
endif()

# the files actually
add_subdirectory(compiler)
add_subdirectory(vm)

add_subdirectory(tests)
enable_testing()

add_test(NAME primal_test COMMAND primal_test)
