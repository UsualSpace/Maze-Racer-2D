# Filename: CMakeLists.txt
# Programmer(s): Abdurrahman Alyajouri
# Date: 4/29/2025
# Purpose: To provide a cross-platform way to build this application

cmake_minimum_required(VERSION 3.10)

project(MazeRacer2D VERSION 0.1.0 LANGUAGES C CXX)

# include library and in-house directories.
include_directories(${CMAKE_SOURCE_DIR}/libraries/glad/include)
include_directories(${CMAKE_SOURCE_DIR}/libraries/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SRC "./source/*.c")
file(GLOB IMGUI_SRC "./libraries/imgui/src/*.cpp")

add_executable(
    MazeRacer2D
    ${SRC}
    ${IMGUI_SRC}
    ./libs/glad/src/glad.c
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(./libraries/glfw)
target_link_libraries(MazeRacer2D glfw)

find_package(OpenGL REQUIRED)
target_link_libraries(MazeRacer2D OpenGL::GL)

set_property(TARGET MazeRacer2D PROPERTY CXX_STANDARD 17)