// Filename: player_queue.h
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/8/2025
// Purpose: To serve as a queue to hold players to form sessions fairly.

#ifndef PLAYER_QUEUE_H
#define PLAYER_QUEUE_H

#include <stddef.h>

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

typedef int player_queue_type_t;

typedef struct player_queue_node {
    player_queue_type_t data;
    struct player_queue_node* next;
} player_queue_node_t;

typedef struct player_queue {
    player_queue_node_t* front;
    player_queue_node_t* back;
    size_t size;  
} player_queue_t;

// initializer/cleanup.
player_queue_t* player_queue_init(void);
int player_queue_free(player_queue_t* queue);

// main api
player_queue_type_t* player_queue_front(player_queue_t* queue);
int player_queue_push(player_queue_t* queue, player_queue_type_t data);
int player_queue_pop(player_queue_t* queue);
int player_queue_clear(player_queue_t* queue);
int player_queue_is_empty(player_queue_t* queue);

#endif //PLAYER_QUEUE_H

