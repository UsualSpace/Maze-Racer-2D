# Maze-Racer-2D
A small networked game serving as our final project for the CS494P Internetworking Protocols course at Portland State University, during the Spring 2025 quarter. An RFC style document that we had to prepare to define our networking protocol is included.

## Setup (WIP description)
This project can be compiled via CMake (see CMakeLists.txt). Make sure to set the appropriate CMake presets for your system (create a CMakePresets.json). It was initially compiled and built using GCC as the compiler and Ninja as the build tool, though other tools will most likely work.

This project uses windows api for sockets, multi-threading, and concurrency so it will only work on windows, but can pretty easily be adapted to other systems.
