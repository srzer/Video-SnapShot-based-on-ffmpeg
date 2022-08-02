add_executable(bench_sample
        ffmpeg_decode_sample.c
        avframe_wrapper.cpp
        avframe_wrapper.hpp
        inheritance.cpp
        template.cpp
        bench.cpp)
target_link_libraries(bench_sample PUBLIC
        FFmpeg::avformat
        FFmpeg::avcodec
        FFmpeg::avutil
        util
        benchmark::benchmark
        )

if (WIN32)
    file(GLOB FFMPEG_DLLS "${FFMPEG_PATH}/bin/*.dll")
    add_custom_command(TARGET bench_sample POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_DLLS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif ()
