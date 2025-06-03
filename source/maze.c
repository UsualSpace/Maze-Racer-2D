// Filename: maze.c
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/9/2025
// Purpose: To implement the api defined in maze_cell.h

#include <string.h>
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
    if(*cell & direction) return FALSE;
    return TRUE;
}

int maze_is_move_valid(maze_t* maze, maze_size_t old_row, maze_size_t old_column, maze_size_t new_row, maze_size_t new_column) {
    //first check if difference between old and new cell coordinates is > 1
    if(abs(new_row - old_row) > 1) return FALSE;
    if(abs(new_column - old_column) > 1) return FALSE;

    //check if move is adjacent vertically or horizontally, diagonals aren't allowed
    if(new_row != old_row && new_column != old_column) {
        fprintf(stderr, "attempted diagonal move.\n");
        return FALSE;
    }

    //check for walls in the moved direction.
    uint8_t direction;

    if(new_row == old_row - 1) {
        direction = NORTH;
    } else if(new_row == old_row + 1) {
        direction = SOUTH;
    } else if(new_column == old_column - 1) {
        direction = WEST;
    } else if(new_column == old_column + 1) {
        direction = EAST;
    } else {
        return FALSE;
    }

    if(maze_cell_check_wall(&maze->cells[old_row][old_column], direction) == TRUE) {
        fprintf(stderr, "attempted to cross a wall.\n");
        return FALSE;
    }

    return TRUE;
}

void backtrack_recursive(int rows, int columns, int row, int column, temp_cell_t** temp_cells) {
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

    //copy only cell component of temp cells structure to maze structure.
    for(int row = 0; row < rows; ++row) {
        for(int column = 0; column < columns; ++column) {
            //printf("(%d, %d) - %d\n", row, column, temp_cells[row][column].cell);
            maze->cells[row][column] = temp_cells[row][column].cell;
        }
    }

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

void print_maze(maze_t* maze) {
    size_t print_rows = 3 * maze->rows;
    size_t print_columns = 3 * maze->columns;
    //printf("%d, %d", maze->rows, maze->columns);
    char wall_char = 'x';

    //allocate space for the printed representation of the maze.
    char** print_maze = malloc(sizeof(char*) * print_rows);
    for(int row = 0; row < print_rows; ++row) {
        print_maze[row] = malloc(print_columns);
        memset(print_maze[row], wall_char, print_columns);
    }

    for(maze_size_t row = 1; row < print_rows; row += 3) {
        for(maze_size_t column = 1; column < print_columns; column += 3) {
            uint8_t directions[4] = { NORTH, SOUTH, EAST, WEST };
            for(int i = 0; i < 4; ++i) {
                if(maze_cell_check_wall(&maze->cells[(row - 1) / 3][(column - 1) / 3], directions[i]) == FALSE) {
                    print_maze[row][column] = ' ';
                    if(directions[i] == NORTH) {
                        print_maze[row - 1][column] = ' ';
                    } else if(directions[i] == SOUTH) {
                        print_maze[row + 1][column] = ' ';
                    } else if(directions[i] == EAST) {
                        print_maze[row][column + 1] = ' ';
                    } else if(directions[i] == WEST) {
                        print_maze[row][column - 1] = ' ';
                    }
                }
            }
        }
    }

    for(size_t row = 0; row < print_rows; ++row) {
        for(size_t column = 0; column < print_columns; ++column) 
            printf("%c ", print_maze[row][column]);
        printf("\n");
    }


    //free the space allocated.
    for(int row = 0; row < print_rows; ++row)
        free(print_maze[row]);
    free(print_maze);

    return;
}

void print_maze_and_players(maze_t* maze, maze_size_t player_one_row, maze_size_t player_one_column, maze_size_t player_two_row, maze_size_t player_two_column) {
    size_t print_rows = 3 * maze->rows;
    size_t print_columns = 3 * maze->columns;
    //printf("%d, %d", maze->rows, maze->columns);
    char wall_char = 'x';
    char player_char = 'o';

    //allocate space for the printed representation of the maze.
    char** print_maze = malloc(sizeof(char*) * print_rows);
    for(int row = 0; row < print_rows; ++row) {
        print_maze[row] = malloc(print_columns);
        memset(print_maze[row], wall_char, print_columns);
    }

    for(maze_size_t row = 1; row < print_rows; row += 3) {
        for(maze_size_t column = 1; column < print_columns; column += 3) {
            uint8_t directions[4] = { NORTH, SOUTH, EAST, WEST };
            for(int i = 0; i < 4; ++i) {
                if(maze_cell_check_wall(&maze->cells[(row - 1) / 3][(column - 1) / 3], directions[i]) == FALSE) {
                    print_maze[row][column] = ' ';
                    if(directions[i] == NORTH) {
                        print_maze[row - 1][column] = ' ';
                    } else if(directions[i] == SOUTH) {
                        print_maze[row + 1][column] = ' ';
                    } else if(directions[i] == EAST) {
                        print_maze[row][column + 1] = ' ';
                    } else if(directions[i] == WEST) {
                        print_maze[row][column - 1] = ' ';
                    }
                }
            }
        }
    }

    print_maze[player_one_row * 3 + 1][player_one_column * 3 + 1] = player_char;
    print_maze[player_two_row * 3 + 1][player_two_column * 3 + 1] = player_char;

    for(size_t row = 0; row < print_rows; ++row) {
        for(size_t column = 0; column < print_columns; ++column) 
            printf("%c ", print_maze[row][column]);
        printf("\n");
    }


    //free the space allocated.
    for(int row = 0; row < print_rows; ++row)
        free(print_maze[row]);
    free(print_maze);

    return;
}