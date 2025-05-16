#include <stdio.h>
#include "maze.h"
#include "maze_cell_stack.h"

int main(void) {
    maze_t* maze = generate_maze(5, 5);

    for(int row = 0; row < 5; ++row) {
        for(int column = 0; column < 5; ++column) {
            printf("%d ", maze->cells[row][column]);
        }
        printf("\n");
    }

    free_maze(maze);

    return 0;
} 