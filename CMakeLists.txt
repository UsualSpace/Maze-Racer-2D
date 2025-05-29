# Filename: CMakeLists.txt
# Programmer(s): Abdurrahman Alyajouri
# Date: 4/29/2025
# Purpose: To provide a cross-platform way to build this application

cmake_minimum_required(VERSION 3.10)

project(MazeRacer2D)

file(GLOB LIBSRC "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c")

list(REMOVE_ITEM LIBSRC
    "${CMAKE_CURRENT_SOURCE_DIR}/source/maze_racer_server.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/maze_racer_test_client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/maze_racer_client.c"
)

add_executable(MazeRacerServer ${CMAKE_CURRENT_SOURCE_DIR}/source/maze_racer_server.c ${LIBSRC})
add_executable(MazeRacerTestClient ${CMAKE_CURRENT_SOURCE_DIR}/source/maze_racer_test_client.c ${LIBSRC})
add_executable(MazeRacerClient ${CMAKE_CURRENT_SOURCE_DIR}/source/maze_racer_client.c ${LIBSRC})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/libraries/glfw)

target_link_libraries(MazeRacerClient glfw)

find_package(OpenGL REQUIRED)

target_include_directories(MazeRacerClient PRIVATE ${CMAKE_SOURCE_DIR}/libraries/glad/include)
target_include_directories(MazeRacerClient PRIVATE ${CMAKE_SOURCE_DIR}/libraries/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

target_link_libraries(MazeRacerClient OpenGL::GL)
target_link_libraries(MazeRacerServer ws2_32)
target_link_libraries(MazeRacerTestClient ws2_32)
target_link_libraries(MazeRacerClient ws2_32)


