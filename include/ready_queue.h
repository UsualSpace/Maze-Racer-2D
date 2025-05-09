#ifndef READY_QUEUE_H
#define READY_QUEUE_H

// Filename: ready_queue.h
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/8/2025
// Purpose: To serve as a queue to hold ready players to form sessions fairly.

#include <stddef.h>

#ifndef TRUE
# define TRUE 1
#endif
#ifndef FALSE
# define FALSE 0
#endif

typedef int ready_queue_type_t;

typedef struct ready_queue_node {
    ready_queue_type_t data;
    struct ready_queue_node* next;
} ready_queue_node_t;

typedef struct ready_queue {
    ready_queue_node_t* front;
    ready_queue_node_t* back;
    size_t size;  
} ready_queue_t;

// initializer/cleanup.
void ready_queue_init(ready_queue_t** queue);
void ready_queue_destroy(ready_queue_t* queue);

// main api
void ready_queue_front(ready_queue_t* queue, ready_queue_type_t* data);
void ready_queue_push(ready_queue_t* queue, ready_queue_type_t data);
void ready_queue_pop(ready_queue_t* queue);
void ready_queue_clear(ready_queue_t* queue);
int ready_queue_isempty(ready_queue_t* queue);

#endif //READY_QUEUE_H

