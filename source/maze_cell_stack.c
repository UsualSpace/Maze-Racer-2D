// Filename: maze_cell_stack.c 
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/8/2025
// Purpose: To implement the api functions defined in maze_cell_stack.h

#include <stdio.h>
#include <stdlib.h>

#include "maze_cell_stack.h" 

maze_cell_stack_t* maze_cell_stack_init(void) {
    maze_cell_stack_t* stack = malloc(sizeof(maze_cell_stack_t));
    if(!stack) {
        perror("failed to initialize maze cell stack");
        return NULL;
    }

    stack->top = NULL;
    stack->size = 0;
    return stack;
}

int maze_cell_stack_free(maze_cell_stack_t* stack) {
    if(!stack) {
        fprintf(stderr, "cannot free an invalid stack\n");
        return ERROR;
    }

    maze_cell_stack_clear(stack);
    free(stack);
    return SUCCESS;
}

maze_cell_stack_type_t maze_cell_stack_top(maze_cell_stack_t* stack) {
    if(!stack) {
        fprintf(stderr, "cannot query top of an invalid stack\n");
        return NULL;
    }

    if(!stack->top) {
        fprintf(stderr, "cannot query top of an empty stack\n");
        return NULL;
    }

    //data is already a pointer.
    return stack->top->data;
}

int maze_cell_stack_push(maze_cell_stack_t* stack, maze_cell_stack_type_t data) {
    if(!stack) {
        fprintf(stderr, "cannot push onto an invalid stack\n");
        return ERROR; 
    }
    
    maze_cell_stack_node_t* new_node = malloc(sizeof(maze_cell_stack_node_t));
    if(!new_node) {
        perror("failed to push to maze cell stack");
        return ERROR;
    }
    
    new_node->data = data;
    new_node->next = NULL;

    if(!stack->top) {
        stack->top = new_node;
    } else {
        maze_cell_stack_node_t* temp = stack->top;
        new_node->next = temp;
        stack->top = new_node;
    }

    ++stack->size;

    printf("pushed\n");
    
    return SUCCESS;
}

int maze_cell_stack_pop(maze_cell_stack_t* stack) {
    if(!stack) {
        fprintf(stderr, "cannot pop an invalid stack\n");
        return ERROR;
    }

    if(!stack->top) {
        fprintf(stderr, "cannot pop an empty stack\n");
        return ERROR;
    }

    maze_cell_stack_node_t* temp_node = stack->top;
    stack->top = temp_node->next;
    free(temp_node);
    --stack->size;

    return SUCCESS;
}

int maze_cell_stack_clear(maze_cell_stack_t* stack) {
    if(!stack) {
        fprintf(stderr, "cannot clear an invalid stack\n");
        return ERROR; 
    }
    
    maze_cell_stack_node_t* current_node = stack->top;
    while(current_node) {
        maze_cell_stack_node_t* next_node = current_node->next;
        free(current_node);
        current_node = next_node;
    }

    stack->top = NULL;
    stack->size = 0;

    return SUCCESS;
}

int maze_cell_stack_is_empty(maze_cell_stack_t* stack) {
    if(!stack) {
        fprintf(stderr, "cannot query emptiness of an invalid stack\n");
        return ERROR;
    }
    if(stack->size > 0) return FALSE;
    return TRUE;
}
