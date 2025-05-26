# Filename: CMakeLists.txt
# Programmer(s): Abdurrahman Alyajouri
# Date: 4/29/2025
# Purpose: To provide a cross-platform way to build this application

cmake_minimum_required(VERSION 3.10)

project(MazeRacer2D)

file(GLOB SRC "./source/*.c")

add_executable(MazeRacer2D ${SRC})

#add_subdirectory(${CMAKE_SOURCE_DIR}/libraries/SDL2)

#target_include_directories(MazeRacer2D PRIVATE ${CMAKE_BINARY_DIR}/libraries/SDL2/include)

target_include_directories(MazeRacer2D PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(MazeRacer2D PRIVATE ws2_32)


