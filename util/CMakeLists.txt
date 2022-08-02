add_library(util STATIC
        PixelConverter.cpp
        PixelConverter.hpp)
target_include_directories(util PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}")
