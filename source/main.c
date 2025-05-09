#include <stdio.h>
#include "../include/ready_queue.h"

void print(ready_queue_t* queue) {
    ready_queue_node_t* cur = queue->front;
    while(cur) {
        printf("%d ", cur->data);
        cur = cur->next;
    }
    printf("\n");
}

int main(void) {
    printf("starting test...\n");

    ready_queue_t* queue = NULL;

    ready_queue_init(&queue);

    printf("init...\n");

    ready_queue_push(queue, 43);
    ready_queue_push(queue, 12);
    ready_queue_push(queue, 0);

    printf("pushed...\n");

    print(queue);

    printf("print...\n");

    ready_queue_pop(queue);

    print(queue);

    int x = 0;
    ready_queue_front(queue, &x);
    printf("%d\n", x);

    ready_queue_clear(queue);

    print(queue);

    ready_queue_push(queue, 4);
    ready_queue_push(queue, 1245);
    ready_queue_push(queue, 98);

    print(queue);

    printf("%d", queue->size);

    ready_queue_destroy(queue);

    return 0;
} 