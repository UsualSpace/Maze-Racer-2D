// Filename: player_queue.c 
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/8/2025
// Purpose: To implement the api functions defined in player_queue.h

#include <stdio.h>
#include <stdlib.h>

#include "player_queue.h" 

player_queue_t* player_queue_init() {
    player_queue_t* queue = malloc(sizeof(player_queue_t));
    if(!queue) {
        perror("failed to initialize player queue");
        return NULL;
    }

    queue->front = queue->back = NULL;
    queue->size = 0;
    return queue;
}

int player_queue_free(player_queue_t* queue) {
    if(!queue) {
        fprintf(stderr, "cannot free an invalid queue\n");
        return ERROR;
    }

    player_queue_clear(queue);
    free(queue);
    return SUCCESS;
}

player_queue_type_t* player_queue_front(player_queue_t* queue) {
    if(!queue) {
        fprintf(stderr, "cannot query front of an invalid queue\n");
        return NULL;
    }

    if(!queue->front) {
        fprintf(stderr, "cannot query front of an empty queue\n");
        return NULL;
    }

    return &queue->front->data;
}

int player_queue_push(player_queue_t* queue, player_queue_type_t data) {
    if(!queue) {
        fprintf(stderr, "cannot push to an invalid queue\n");
        return ERROR; 
    }

    player_queue_node_t* new_node = malloc(sizeof(player_queue_node_t));
    if(!new_node) {
        perror("failed to push to player queue");
        return ERROR;
    }
    
    new_node->data = data;
    new_node->next = NULL;

    if(!queue->front) {
        queue->front = new_node;
        queue->back = new_node;
    } else {
        queue->back->next = new_node;
        queue->back = new_node;
    }

    ++queue->size;
    
    return SUCCESS;
}

int player_queue_pop(player_queue_t* queue) {
    if(!queue) {
        fprintf(stderr, "cannot pop an invalid queue\n");
        return ERROR;
    }

    if(!queue->front) {
        fprintf(stderr, "cannot pop an empty queue\n");
        return ERROR;
    }

    player_queue_node_t* temp_node = queue->front;
    queue->front = temp_node->next;
    free(temp_node);
    --queue->size;

    return SUCCESS;
}

int player_queue_clear(player_queue_t* queue) {
    if(!queue) {
        fprintf(stderr, "cannot clear an invalid queue\n");
        return ERROR; 
    }
    
    player_queue_node_t* current_node = queue->front;
    while(current_node) {
        player_queue_node_t* next_node = current_node->next;
        free(current_node);
        current_node = next_node;
    }

    queue->front = queue->back = NULL;
    queue->size = 0;

    return SUCCESS;
}

int player_queue_is_empty(player_queue_t* queue) {
    if(!queue) {
        fprintf(stderr, "cannot query emptiness of an invalid stack\n");
        return ERROR;
    }

    if(queue->size > 0) return FALSE;
    return TRUE;
}
