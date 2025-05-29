// Filename: networking_utils.h
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/16/2025
// Purpose: To define the various messages the application intends to send and receive, and provide a toolset to send and receive them.

#ifndef NETWORKING_UTILS_H
# define NETWORKING_UTILS_H

#include <stdint.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "maze.h"

//default server/client properties. TODO: initialize through cli arguments.
#define MRMP_DEFAULT_PORT "9898"

//helper error codes.
#define GRACEFUL_DC                     (-1)
#define DISGRACEFUL_DC                  (-2)
#define TIMEDOUT                        (-3)

//opcodes.
#define MRMP_OPCODE_ERROR 			    0b00000000
#define MRMP_OPCODE_PING 			    0b00000001
#define MRMP_OPCODE_HELLO 			    0b00000010
#define MRMP_OPCODE_JOIN 			    0b00000011
#define MRMP_OPCODE_LEAVE 			    0b00000100
#define MRMP_OPCODE_MOVE 			    0b00000101
#define MRMP_OPCODE_BAD_MOVE 	        0b00000110
#define MRMP_OPCODE_RESULT 			    0b00000111
#define MRMP_OPCODE_JOIN_RESP		    0b00001000
#define MRMP_OPCODE_START 			    0b00001001
#define MRMP_OPCODE_READY 			    0b00001010
#define MRMP_OPCODE_PONG 			    0b00001011
#define MRMP_OPCODE_TIMEOUT 		    0b00001100
#define MRMP_OPCODE_OPPONENT_MOVE	    0b00001101
#define MRMP_OPCODE_HELLO_ACK 			0b00001110

//error codes.
#define  MRMP_ERR_UNKNOWN               0b00000000
#define  MRMP_ERR_ILLEGAL_OPCODE        0b00000001
#define  MRMP_ERR_VERSION_MISMATCH      0b00000010
#define  MRMP_ERR_FULL_QUEUE            0b00000011

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

//Can be replaced by mrmp_pkt_move with a different opcode.
// typedef struct mrmp_pkt_bad_move {
//     mrmp_pkt_header_t header;
//     maze_size_t last_row;
//     maze_size_t last_column;
// } mrmp_pkt_bad_move_t; 

// typedef struct mrmp_pkt_opponent_move {
//     mrmp_pkt_header_t header;
//     maze_size_t row;
//     maze_size_t column;
// } mrmp_pkt_bad_move_t; 

typedef struct mrmp_pkt_result {
    mrmp_pkt_header_t header;
    mrmp_winner_t winner;
} mrmp_pkt_bad_move_t; 

typedef struct session {
    SOCKET player_one;
    SOCKET player_two;
    uint8_t player_one_row;
    uint8_t player_one_column;
    uint8_t player_two_row;
    uint8_t player_two_column;
} session_t;

int send_error_pkt(SOCKET socket, mrmp_error_t error);
int send_hello_pkt(SOCKET socket, mrmp_version_t version);
int send_hello_ack_pkt(SOCKET socket);
int send_join_pkt(SOCKET socket);
int send_join_resp_pkt(SOCKET socket, maze_t* maze);
int send_leave_pkt(SOCKET socket);
int send_move_pkt(SOCKET socket, maze_size_t row, maze_size_t column);
int send_opponent_move_pkt(SOCKET socket, maze_size_t row, maze_size_t column);
int send_bad_move_pkt(SOCKET socket, maze_size_t last_row, maze_size_t last_column);
int send_timeout_pkt(SOCKET socket);

int process_error_pkt();
int process_hello_pkt();
int process_join_pkt();
int process_leave_pkt(SOCKET socket, session_t* session, mrmp_pkt_move_t* pkt);
int process_move_pkt(SOCKET socket, session_t* session, mrmp_pkt_move_t* pkt);
int process_bad_move_pkt(SOCKET socket, session_t* session);

int recv_w_timeout(SOCKET s, DWORD timeout_ms);
int receive_mrmp_msg(SOCKET socket, void** out_msg);

#endif //NETWORKING_UTILS_H
