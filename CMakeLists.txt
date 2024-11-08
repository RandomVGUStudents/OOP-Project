cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Tetris)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -g")

# Find the raylib package
find_package(raylib REQUIRED)

# List all source files
set(SOURCES
    src/core/block.cpp
    src/core/board.cpp
    src/core/tetris.cpp
    src/ui/mainUI.cpp
    src/ui/tetrisUI.cpp
    src/ui/renderer.cpp
    src/ui/cli.cpp
    src/ai/env.cpp
    src/ai/heuristics.cpp
    src/ai/genetic.cpp
    src/main.cpp
)

# Add the executable
add_executable(Tetris ${SOURCES})

# Link the raylib library
target_link_libraries(Tetris PRIVATE raylib)

# Optionally, set the build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Include directories (if needed)
target_include_directories(Tetris PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

