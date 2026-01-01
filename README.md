# Maze-Racer-2D
A small networked game serving as our final project for the CS494P Internetworking Protocols course at Portland State University, during the Spring 2025 quarter. An RFC style document that we had to prepare to define our networking protocol is included. NOTE: the provided code is functional, but some small details may be incomplete or inconsistent. I hope to come back in the future and polish the code. 

## Setup
This project can be compiled via CMake (see CMakeLists.txt). Make sure to set the appropriate CMake presets for your system (create and configure a CMakePresets.json). It was initially compiled and built using GCC as the compiler and Ninja as the build tool, though other tools will most likely work.

This project's implementation of the Maze Racer Multiplayer Protocol (MRMP) uses the windows api for sockets, multi-threading, and concurrency so it will only work on windows, but can pretty easily be adapted to other systems.

## Execution
After compiling, run the server process on a windows machine with ```./MazeRacerServer.exe```. A help message will be displayed to guide you on what you can do. Run the client process on a windows machine and pass in the IP to the machine running the server process along with the port, which by default is set to ```9898```. An example run of the client could be: ```./MazeRacerClient.exe 10.10.10.10 9898```. After at least 2 successful client connections are made to the server, both clients will be matched and each be sent a text based visualization of the maze. Each client will be represented with the character 'o' and the goal is to reach the bottom right cell of the maze first. What this should look like:

<img width="1659" height="393" alt="image" src="https://github.com/user-attachments/assets/cefc123b-fb7d-4e45-adfc-208d3e82e2bb" />

*There are 3 terminals shown here. Left: The server's minimal user interface to check the status of things. Middle: One of the two paired clients playing a match. Right: The other of the two paired clients playing a match*

The keyboard controls are:
* W - UP
* A - LEFT
* S - DOWN
* D - RIGHT    

