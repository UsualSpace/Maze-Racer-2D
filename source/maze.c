// Filename: maze.c
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/9/2025
// Purpose: To implement the api defined in maze_cell.h

#include <stdlib.h>

#include "maze.h"
#include "maze_cell_stack.h"

int maze_cell_remove_wall(maze_cell_t* cell, uint8_t direction) {
    *cell &= direction;
}

int maze_cell_check_wall(maze_cell_t* cell, uint8_t direction) {
    if(*cell & direction) return TRUE;
    return FALSE;
}

maze_t* generate_maze(maze_size_t rows, maze_size_t columns) {
    //allocate maze array and related structures.
    maze_t* maze = malloc(sizeof(maze_t));
    maze->rows = rows;
    maze->columns = columns;

    struct temp_cell** temp_cells = malloc(sizeof(temp_cell_t*) * rows);
    maze_cell_t** cells = malloc(sizeof(maze_cell_t*) * rows);
    for(int row = 0; row < rows; ++row) {
        temp_cells[row] = malloc(sizeof(temp_cell_t) * columns);
        cells = malloc(sizeof(maze_cell_t) * columns);
    }

    //initialize maze array.
    for(int row = 0; row < rows; ++row) {
        for(int column = 0; column < columns; ++column) {
            temp_cells[row][column].visited = FALSE;
        }
    }

    maze_cell_stack_t* cell_stack = maze_cell_stack_init();
    maze_cell_stack_push(cell_stack, &temp_cells[0][0]);
    
    int current_row = 0;
    int current_column = 0;
    temp_cell_t* current_cell = NULL;
    while((current_cell = maze_cell_stack_top(cell_stack))->visited == FALSE) {
        //mark cell as visited.
        current_cell->visited = TRUE;

        uint8_t direction;
        int new_row = 0;
        int new_column = 0;
        switch(rand() % 4) {
            case 0:
                direction = NORTH;
                new_row -= 1;
                break;
            case 1: 
                direction = SOUTH;
                new_row += 1;
                break;
            case 2:
                direction = EAST;
                new_column += 1;
                break;
            case 3: 
                direction = WEST;
                new_column -= 1;
                break;
            default:
                break;
        }

        if((new_row >= 0 && new_row < rows) && (new_column >= 0 && new_column < columns)) {
            
        }
    }

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