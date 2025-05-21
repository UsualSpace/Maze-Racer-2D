// Filename: networking_utils.h
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/16/2025
// Purpose: To define the various messages the application intends to send and receive, and provide a toolset to send and receive them.

#include <stdint.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "maze.h"

typedef uint8_t mrmp_opcode_t;
typedef uint32_t mrmp_payload_size_t;
typedef uint8_t mrmp_version_t;
typedef uint8_t mrmp_error_t;
typedef uint8_t mrmp_winner_t;

//sufficient for join, leave, bad move packets.
typedef struct mrmp_pkt_header {
    mrmp_opcode_t opcode;
    mrmp_payload_size_t length;
} mrmp_pkt_header_t;

typedef struct mrmp_pkt_error {
    mrmp_pkt_header_t header;
    mrmp_error_t error_code;
} mrmp_pkt_error_t;

typedef struct mrmp_pkt_hello {
    mrmp_pkt_header_t header;
    mrmp_version_t version;
} mrmp_pkt_hello_t; 

typedef struct mrmp_pkt_join_resp {
    mrmp_pkt_header_t header;
    maze_size_t rows;
    maze_size_t columns;
    maze_cell_t cells[];
} mrmp_pkt_join_resp_t; 

typedef struct mrmp_pkt_move {
    mrmp_pkt_header_t header;
    maze_size_t row;
    maze_size_t column;
} mrmp_pkt_move_t; 

typedef struct mrmp_pkt_bad_move {
    mrmp_pkt_header_t header;
    maze_size_t last_row;
    maze_size_t last_column;
} mrmp_pkt_bad_move_t; 

typedef struct mrmp_pkt_opponent_move {
    mrmp_pkt_header_t header;
    maze_size_t row;
    maze_size_t column;
} mrmp_pkt_bad_move_t; 

typedef struct mrmp_pkt_result {
    mrmp_pkt_header_t header;
    mrmp_winner_t winner;
} mrmp_pkt_bad_move_t; 

typedef struct session_state {
    SOCKET player_one;
    SOCKET player_two;
    uint8_t player_one_row;
    uint8_t player_one_column;
    uint8_t player_two_row;
    uint8_t player_two_column;
} session_state_t;

int send_error_pkt();
int send_hello_pkt();
int send_join_pkt();
int send_join_resp_pkt();
int send_leave_pkt();
int send_move_pkt();
int send_bad_move_pkt();

int handle_error_pkt();
int handle_hello_pkt();
int handle_join_pkt();
int handle_leave_pkt();
int handle_move_pkt();
int handle_bad_move_pkt();

void accept_server_message(void* client_state);
void accept_client_message(void* session_state);

