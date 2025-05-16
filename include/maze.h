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

//a cell will be defined as a bitset representing surrounding walls.
typedef uint8_t maze_cell_t;

//assuming it is unsafe to send size_t over the internet, so using a type with guaranteed size.
typedef uint8_t maze_size_t;

typedef struct maze {
    maze_size_t rows;
    maze_size_t columns;
    maze_cell_t** cells;
} maze_t;

typedef struct temp_cell {
    maze_cell_t cell;
    int visited;
} temp_cell_t;

//randomly shuffle a static array of 8 bit integers.
void shuffle(uint8_t array[], int length);

//there is probably a better way to handle this.
//returns the complement of the given direction.
uint8_t complement(uint8_t direction);

//given the number of rows and columns of a maze, and a specific cell's row/column coordinate, return
//TRUE if the cell coordinate is valid or FALSE otherwise.
int maze_cell_is_valid(maze_size_t rows, maze_size_t columns, int row, int column);

//given a cell pointer and a direction bit mask, will remove a wall in the given direction.
//returns ERROR if something went wrong and SUCCESS otherwise.
int maze_cell_remove_wall(maze_cell_t* cell, uint8_t direction);

//given a cell pointer and a direction bit mask, returns FALSE if no wall exists in
//the given direction and TRUE if a wall does exist.
int maze_cell_check_wall(maze_cell_t* cell, uint8_t direction);

void backtrack_recursive(int rows, int columns, int row, int column, temp_cell_t** temp_cells);

//given rows and columns, will return a randomly generated 2D maze of
//rows * columns cells, using recursive backtracking.
maze_t* generate_maze(maze_size_t rows, maze_size_t columns);

//given a valid 2D maze cell array, will deallocate/free everything.
int free_maze(maze_t* maze);

#endif //MAZE_CELL_H