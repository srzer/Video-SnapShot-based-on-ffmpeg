cmake_minimum_required(VERSION 3.21)
project(project2)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo"
            CACHE STRING
            "Build Type [Release, Debug, RelWithDebInfo]" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            Release RelWithDebInfo Debug)
endif ()

if (WIN32)
    set(FFMPEG_PATH "${CMAKE_CURRENT_SOURCE_DIR}\\external\\ffmpeg-windows")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}\\exes")
endif ()

find_package(FFmpeg REQUIRED COMPONENTS avformat avcodec avutil swscale)

add_subdirectory(external)

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED)

if (IPO_SUPPORTED)
    message("-- IPO Supported, enabling in release mode")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
endif ()

file(REAL_PATH "samples" TEA_PROJECT_SAMPLE_PATH)
add_compile_definitions(TEA_PROJECT_SAMPLE_PATH="${TEA_PROJECT_SAMPLE_PATH}")

add_subdirectory(util)
add_subdirectory(microbench)
add_subdirectory(src)
