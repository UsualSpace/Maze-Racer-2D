#include <stdio.h>
#include "../include/player_queue.h"

void print(player_queue_t* queue) {
    player_queue_node_t* cur = queue->front;
    while(cur) {
        printf("%d ", cur->data);
        cur = cur->next;
    }
    printf("\n");
}

int main(void) {
    printf("starting test...\n");

    player_queue_t* queue = NULL;

    printf("%d\n", player_queue_init(&queue));

    printf("init...\n");

    printf("%d\n", player_queue_push(queue, 43));
    player_queue_push(queue, 12);
    player_queue_push(queue, 0);

    printf("pushed...\n");

    print(queue);

    printf("print...\n");

    player_queue_pop(queue);

    print(queue);

    int x = 0;
    player_queue_front(queue, &x);
    printf("%d\n", x);

    player_queue_clear(queue);

    print(queue);

    player_queue_push(queue, 4);
    player_queue_push(queue, 1245);
    player_queue_push(queue, 98);

    print(queue);

    printf("%d", queue->size);

    player_queue_free(queue);

    return 0;
} 