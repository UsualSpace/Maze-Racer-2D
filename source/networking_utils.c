// Filename: networking_utils.c
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/18/2025
// Purpose: 

#include "networking_utils.h"

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

//error codes.
#define  MRMP_ERR_UNKNOWN               0b00000000
#define  MRMP_ERR_ILLEGAL_OPCODE        0b00000001
#define  MRMP_ERR_VERSION_MISMATCH      0b00000010
#define  MRMP_ERR_FULL_QUEUE            0b00000011

//error strings.
const char* code_to_error[] = {
    "unknown error occurred at server",
    "illegal operation code used",
    "version mismatch",
    "player queue has reached maximum capacity"
};

// void fetch_server_message(void* client_state);

// void handle_session_message(int player, void* session_state) {
    
// }

