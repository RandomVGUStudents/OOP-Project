cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Tetris)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

# Find the raylib package
find_package(raylib REQUIRED)

# List all source files
set(SOURCES
  #src/animation.cpp
    src/block.cpp
    src/board.cpp
    src/game.cpp
    src/main.cpp
    src/ui.cpp
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

