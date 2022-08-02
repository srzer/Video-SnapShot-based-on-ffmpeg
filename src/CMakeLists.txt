add_executable(main
        VideoContactSheet.cpp
        main.cpp)
target_link_libraries(main PUBLIC
        FFmpeg::swscale
        FFmpeg::avformat
        FFmpeg::avcodec
        FFmpeg::avutil
        util
        )

if (WIN32)
    file(GLOB FFMPEG_DLLS "${FFMPEG_PATH}/bin/*.dll")
    add_custom_command(TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_DLLS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif ()
