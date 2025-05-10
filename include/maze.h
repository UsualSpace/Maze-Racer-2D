// Filename: maze.h
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/9/2025
// Purpose: To define what a cell is, in regards to a maze.

#ifndef MAZE_CELL_H
#define MAZE_CELL_H

#include <stdint.h>
#include <stddef.h>

#ifndef TRUE
# define TRUE 1
#endif //TRUE
#ifndef FALSE
# define FALSE 0
#endif //FALSE

#ifndef SUCCESS
# define SUCCESS 0
#endif //SUCCESS
#ifndef ERROR
# define ERROR 1
#endif //ERROR

//direction bitmasks, 1 indicates no wall in a specific direction, 0 indicates a wall in a specific direction.
const uint8_t NORTH = 0b00000001;
const uint8_t SOUTH = 0b00000010;
const uint8_t EAST  = 0b00000100;
const uint8_t WEST  = 0b00001000;

//a cell will be defined as a bitset of surrounding walls.
typedef uint8_t maze_cell_t;

//assuming it is unsafe to send size_t over the internet, so using a type with guaranteed size.
typedef uint32_t maze_size_t;

typedef struct maze {
    maze_size_t rows;
    maze_size_t columns;
    maze_cell_t** cells;
} maze_t;

typedef struct temp_cell {
    maze_cell_t cell;
    int visited;
} temp_cell_t;

//given a cell pointer and a direction bit mask, will remove a wall in the given direction.
//returns ERROR if something went wrong and SUCCESS otherwise.
int maze_cell_remove_wall(maze_cell_t* cell, uint8_t direction);

//given a cell pointer and a direction bit mask, returns FALSE if no wall exists in
//the given direction and TRUE if a wall does exist.
int maze_cell_check_wall(maze_cell_t* cell, uint8_t direction);

//given rows and columns, will return a randomly generated 2D maze of
//rows * columns cells, using recursive backtracking.
maze_t* generate_maze(maze_size_t rows, maze_size_t columns);

//given a valid 2D maze cell array, will deallocate/free everything.
int free_maze(maze_t* maze);

#endif //MAZE_CELL_H