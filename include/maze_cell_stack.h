// Filename: maze_cell_stack.h
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/9/2025
// Purpose: To serve as a stack to hold maze cells to assist in generating random mazes.

#ifndef MAZE_CELL_STACK_H
#define MAZE_CELL_STACK_H

#include <stddef.h>

#include "maze.h"

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

typedef temp_cell_t* maze_cell_stack_type_t;

typedef struct maze_cell_stack_node {
    maze_cell_stack_type_t data;
    struct maze_cell_stack_node* next;
} maze_cell_stack_node_t;

typedef struct maze_cell_stack {
    maze_cell_stack_node_t* top;
    size_t size;  
} maze_cell_stack_t;

// initializer/cleanup.
maze_cell_stack_t* maze_cell_stack_init(void);
int maze_cell_stack_free(maze_cell_stack_t* stack);

// main api
maze_cell_stack_type_t maze_cell_stack_top(maze_cell_stack_t* stack);
int maze_cell_stack_push(maze_cell_stack_t* stack, maze_cell_stack_type_t data);
int maze_cell_stack_pop(maze_cell_stack_t* stack);
int maze_cell_stack_clear(maze_cell_stack_t* stack);
int maze_cell_stack_is_empty(maze_cell_stack_t* stack);

#endif //MAZE_CELL_STACK_H

