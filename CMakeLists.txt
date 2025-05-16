# Filename: CMakeLists.txt
# Programmer(s): Abdurrahman Alyajouri
# Date: 4/29/2025
# Purpose: To provide a cross-platform way to build this application

cmake_minimum_required(VERSION 3.10)

project(MazeRacer2D)

# include library and in-house directories.

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SRC "./source/*.c")

add_executable(MazeRacer2D ${SRC})
