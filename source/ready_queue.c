// Filename: ready_queue.c 
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/8/2025
// Purpose: To implement the api functions defined in ready_queue.h

#include <stdio.h>
#include <stdlib.h>

#include "../include/ready_queue.h" 

void ready_queue_init(ready_queue_t** queue) {
    *queue = malloc(sizeof(ready_queue_t));
    if(!*queue) {
        //TODO: error handle.
    }

    (*queue)->front = (*queue)->back = NULL;
    (*queue)->size = 0;
}

void ready_queue_destroy(ready_queue_t* queue) {
    if(!queue) return;
    ready_queue_clear(queue);
    free(queue);
}

void ready_queue_front(ready_queue_t* queue, ready_queue_type_t* data) {
    if(!queue) return; 
    if(!ready_queue_isempty(queue))
        *data = queue->front->data;
}

void ready_queue_push(ready_queue_t* queue, ready_queue_type_t data) {
    if(!queue) return; 
    
    ready_queue_node_t* new_node = malloc(sizeof(ready_queue_node_t));
    if(!new_node) {
        //TODO: error handle.
    }
    
    new_node->data = data;
    new_node->next = NULL;

    if(ready_queue_isempty(queue)) {
        queue->front = new_node;
        queue->back = new_node;
    } else {
        queue->back->next = new_node;
        queue->back = new_node;
    }

    ++queue->size;
}

void ready_queue_pop(ready_queue_t* queue) {
    if(!queue) return;
    if(ready_queue_isempty(queue)) return;
    
    ready_queue_node_t* temp_node = queue->front;
    queue->front = temp_node->next;
    free(temp_node);
    --queue->size;
}

void ready_queue_clear(ready_queue_t* queue) {
    if(!queue) return; 
    
    ready_queue_node_t* current_node = queue->front;
    while(current_node) {
        ready_queue_node_t* next_node = current_node->next;
        free(current_node);
        current_node = next_node;
    }

    queue->front = queue->back = NULL;
    queue->size = 0;
}

int ready_queue_isempty(ready_queue_t* queue) {
    if(!queue) return -1;
    if(queue->size > 0) return FALSE;
    return TRUE;
}
