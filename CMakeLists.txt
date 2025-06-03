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
#add_executable(MazeRacerClient ${CMAKE_CURRENT_SOURCE_DIR}/source/maze_racer_client.c ${LIBSRC})

include_directories(${CMAKE_SOURCE_DIR}/include)

target_link_libraries(MazeRacerServer ws2_32)
target_link_libraries(MazeRacerTestClient ws2_32)
#target_link_libraries(MazeRacerClient ws2_32 OpenGL::GL)


