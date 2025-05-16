// Filename: maze.c
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/9/2025
// Purpose: To implement the api defined in maze_cell.h

#include <stdio.h>
#include <stdlib.h>

#include "maze.h"
#include "maze_cell_stack.h"

//direction bitmasks, 1 indicates no wall in a specific direction, 0 indicates a wall in a specific direction.
const uint8_t NORTH = 0b00000001;
const uint8_t SOUTH = 0b00000010;
const uint8_t EAST  = 0b00000100;
const uint8_t WEST  = 0b00001000;

void shuffle(uint8_t array[], int length) {
    for(int i = 0; i < length; ++i) {
        int random_idx = rand() % length;
        uint8_t temp = array[random_idx];
        array[random_idx] = array[i];
        array[i] = temp;
    }
}

uint8_t complement(uint8_t direction) {
    if(direction == NORTH) {
        return SOUTH;
    } else if(direction == SOUTH) {
        return NORTH;
    } else if(direction == EAST) {
        return WEST;
    } else if(direction == WEST) {
        return EAST;
    }

    return direction;
}

int maze_cell_is_valid(maze_size_t rows, maze_size_t columns, int row, int column) {
    if((row >= 0 && row < rows) && (column >= 0 && column < columns)) return TRUE;
    return FALSE;
}

int maze_cell_remove_wall(maze_cell_t* cell, uint8_t direction) {
    *cell |= direction;
}

int maze_cell_check_wall(maze_cell_t* cell, uint8_t direction) {
    if(*cell & direction) return TRUE;
    return FALSE;
}

void backtrack_recursive(int rows, int columns, int row, int column, temp_cell_t** temp_cells) {
    printf("recursed, (%d, %d)\n", row, column);
    temp_cell_t* current_cell = &temp_cells[row][column];

    //mark cell as visited.
    current_cell->visited = TRUE;

    //check all neighbors for visitation status.
    uint8_t directions[4] = {NORTH, SOUTH, EAST, WEST};
    shuffle(directions, 4);
    for(int i = 0; i < 4; ++i) {
        int new_row = row;
        int new_column = column;
        if(directions[i] == NORTH) {
            new_row -= 1;
        } else if(directions[i] == SOUTH) {
            new_row += 1;
        } else if(directions[i] == EAST) {
            new_column += 1;
        } else if(directions[i] == WEST) {
            new_column -= 1;
        }

        if(maze_cell_is_valid(rows, columns, new_row, new_column) && temp_cells[new_row][new_column].visited == FALSE) {
            maze_cell_remove_wall(&current_cell->cell, directions[i]);
            maze_cell_remove_wall(&temp_cells[new_row][new_column].cell, complement(directions[i]));
            backtrack_recursive(rows, columns, new_row, new_column, temp_cells);
        }
    }
}

maze_t* generate_maze(maze_size_t rows, maze_size_t columns) {
    //allocate maze array and related structures.
    maze_t* maze = malloc(sizeof(maze_t));
    maze->rows = rows;
    maze->columns = columns;
    maze->cells = malloc(sizeof(maze_cell_t*) * rows);

    temp_cell_t** temp_cells = malloc(sizeof(temp_cell_t*) * rows);
    for(int row = 0; row < rows; ++row) {
        temp_cells[row] = malloc(sizeof(temp_cell_t) * columns);
        maze->cells[row] = malloc(sizeof(maze_cell_t) * columns);
    }

    //initialize maze array.
    for(int row = 0; row < rows; ++row) {
        for(int column = 0; column < columns; ++column) {
            temp_cells[row][column].cell = 0;
            temp_cells[row][column].visited = FALSE;
        }
    }

    //start at the top left corner of the maze, (0, 0).
    backtrack_recursive(rows, columns, 0, 0, temp_cells);

    printf("finished recursion\n");

    //copy only cell component of temp cells structure to maze structure.
    for(int row = 0; row < rows; ++row) {
        for(int column = 0; column < columns; ++column) {
            printf("(%d, %d) - %d\n", row, column, temp_cells[row][column].cell);
            maze->cells[row][column] = temp_cells[row][column].cell;
        }
    }

    printf("reached\n");

    for(int row = 0; row < rows; ++row)
        free(temp_cells[row]);
    free(temp_cells);

    return maze;
}

int free_maze(maze_t* maze) {
    for(maze_size_t row = 0; row < maze->rows; ++row) {
        free(maze->cells[row]);
    }

    free(maze->cells);
    free(maze);
    
    return SUCCESS;
}