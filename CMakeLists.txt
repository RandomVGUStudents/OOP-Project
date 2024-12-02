cmake_minimum_required(VERSION 3.10)

project(Tetris)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -g -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(raylib REQUIRED)

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

add_executable(Tetris ${SOURCES})
target_link_libraries(Tetris PRIVATE raylib)
target_include_directories(Tetris PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
