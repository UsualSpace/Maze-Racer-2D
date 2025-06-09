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

//default server/client properties.
#define MRMP_DEFAULT_PORT "9898"

//helper error codes.
#define GRACEFUL_DC                     (-1)
#define DISGRACEFUL_DC                  (-2)
#define TIMEDOUT                        (-3)

//opcodes.
#define MRMP_OPCODE_ERROR 			    0b00000001
#define MRMP_OPCODE_HELLO 			    0b00000010
#define MRMP_OPCODE_JOIN 			    0b00000011
#define MRMP_OPCODE_LEAVE 			    0b00000100
#define MRMP_OPCODE_MOVE 			    0b00000101
#define MRMP_OPCODE_BAD_MOVE 	        0b00000110
#define MRMP_OPCODE_RESULT 			    0b00000111
#define MRMP_OPCODE_JOIN_RESP		    0b00001000
#define MRMP_OPCODE_START 			    0b00001001
#define MRMP_OPCODE_READY 			    0b00001010
#define MRMP_OPCODE_TIMEOUT 		    0b00001011
#define MRMP_OPCODE_OPPONENT_MOVE	    0b00001100
#define MRMP_OPCODE_HELLO_ACK 			0b00001101

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

#define PHEADER(msg) ((mrmp_pkt_header_t*)(msg))
#define PMOVE(msg)   ((mrmp_pkt_move_t*)(msg))
#define PJOINRE(msg) ((mrmp_pkt_join_resp_t*)(msg))
#define PRESULT(msg) ((mrmp_pkt_result_t*)(msg))
#define PHELLO(msg)  ((mrmp_pkt_hello_t*)(msg))

//manually maintain tightly packed sizes of structs due to struct padding throwing off sizes.
#define MRMP_PKT_HEADER_SIZE (sizeof(mrmp_opcode_t) + sizeof(mrmp_payload_size_t))
#define MRMP_PKT_ERROR_SIZE (MRMP_PKT_HEADER_SIZE + sizeof(mrmp_error_t))
#define MRMP_PKT_HELLO_SIZE (MRMP_PKT_HEADER_SIZE + sizeof(mrmp_version_t))
#define MRMP_PKT_JOIN_RESP_PARTIAL_SIZE (MRMP_HEADER_SIZE + sizeof(maze_size_t) * 2) //size of maze isnt known at compile time.
#define MRMP_PKT_MOVE_SIZE (MRMP_PKT_HEADER_SIZE + sizeof(maze_size_t) * 2)
#define MRMP_PKT_RESULT_SIZE (MRMP_PKT_HEADER_SIZE + sizeof(mrmp_winner_t))

//#pragma pack(push, 1) //easy way out, less portable
 
//sufficient for join, leave, bad move packets, etc.
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

//#pragma pack(pop) //easy way out, less portable

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
} mrmp_pkt_result_t; 

int send_buffer(SOCKET socket, const char* buffer, int buffer_length);
char* buffer_to_mrmp_pkt_struct(char* buffer);

int send_error_pkt(SOCKET socket, mrmp_error_t error);
int send_hello_pkt(SOCKET socket, mrmp_version_t version);
int send_hello_ack_pkt(SOCKET socket);
int send_join_pkt(SOCKET socket);
int send_join_resp_pkt(SOCKET socket, maze_t* maze);
int send_ready_pkt(SOCKET socket);
int send_start_pkt(SOCKET socket);
int send_leave_pkt(SOCKET socket);
int send_move_pkt(SOCKET socket, maze_size_t row, maze_size_t column);
int send_opponent_move_pkt(SOCKET socket, maze_size_t row, maze_size_t column);
int send_bad_move_pkt(SOCKET socket, maze_size_t last_row, maze_size_t last_column);
int send_result_pkt(SOCKET socket, mrmp_winner_t winner);
int send_timeout_pkt(SOCKET socket);

maze_t* maze_network_to_host(mrmp_pkt_join_resp_t* msg);

int recv_w_timeout(SOCKET socket, char* buffer, int length, int flags, struct timeval* timeout);
int receive_mrmp_msg(SOCKET socket, char** out_msg, struct timeval* timeout);

#endif //NETWORKING_UTILS_H
