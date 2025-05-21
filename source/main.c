#include <stdio.h>
#include "maze.h"
#include "maze_cell_stack.h"

int main(void) {
    maze_t* maze = generate_maze(20, 20);

    print_maze(maze);

    free_maze(maze);

    return 0;
} 