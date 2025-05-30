// Filename: networking_utils.c
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/18/2025
// Purpose: 

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "networking_utils.h"

//error strings.
const char* code_to_error[] = {
    "unknown error occurred at server",
    "illegal operation code used",
    "version mismatch",
    "player queue has reached maximum capacity"
};

int send_buffer(SOCKET socket, char* buffer, size_t buffer_length) {
    int total_bytes_sent = 0;
    int bytes_expected = buffer_length;

    while(total_bytes_sent < bytes_expected) {
        int bytes_sent = send(socket, (const char*) buffer + total_bytes_sent, bytes_expected - total_bytes_sent, 0);
        if(bytes_sent == SOCKET_ERROR) {
            return SOCKET_ERROR;
        }
        total_bytes_sent += bytes_sent;
    }

    return SUCCESS;
}

int send_error_pkt(SOCKET socket, mrmp_error_t error) {
    mrmp_pkt_error_t msg = {
        {
            .opcode = MRMP_OPCODE_ERROR,
            .length = sizeof(mrmp_error_t)
        },
        .error_code = error
    };

    int total_bytes_sent = 0;
    int bytes_expected = sizeof(msg);

    while(total_bytes_sent < bytes_expected) {
        int bytes_sent = send(socket, (const char*) &msg + total_bytes_sent, bytes_expected - total_bytes_sent, 0);
        if(bytes_sent == SOCKET_ERROR) {
            fprintf(stderr, "failed to send error packet\n");
            return DISGRACEFUL_DC;
        }
        total_bytes_sent += bytes_sent;
    }

    return SUCCESS;
}

int send_hello_pkt(SOCKET socket, mrmp_version_t version) {
    mrmp_pkt_hello_t msg = {
        {
            .opcode = MRMP_OPCODE_HELLO,
            .length = sizeof(mrmp_version_t)
        },
        .version = version
    };

    int total_bytes_sent = 0;
    int bytes_expected = sizeof(msg);

    while(total_bytes_sent < bytes_expected) {
        int bytes_sent = send(socket, (const char*) &msg + total_bytes_sent, bytes_expected - total_bytes_sent, 0);
        if(bytes_sent == SOCKET_ERROR) {
            fprintf(stderr, "failed to send hello packet\n");
            return DISGRACEFUL_DC;
        }
        
        total_bytes_sent += bytes_sent;
    }

    return SUCCESS;
}

int send_hello_ack_pkt(SOCKET socket) {
    mrmp_pkt_header_t msg = {
        .opcode = MRMP_OPCODE_HELLO_ACK,
        .length = 0
    };

    int total_bytes_sent = 0;
    int bytes_expected = sizeof(msg);

    while(total_bytes_sent < bytes_expected) {
        int bytes_sent = send(socket, (const char*) &msg + total_bytes_sent, bytes_expected - total_bytes_sent, 0);
        if(bytes_sent == SOCKET_ERROR) {
            fprintf(stderr, "failed to send hello acknowledgement packet\n");
            return DISGRACEFUL_DC;
        }
        total_bytes_sent += bytes_sent;
    }

    return SUCCESS;
}

int send_join_pkt(SOCKET socket);
int send_join_resp_pkt(SOCKET socket, maze_t* maze);
int send_leave_pkt(SOCKET socket);
int send_move_pkt(SOCKET socket, maze_size_t row, maze_size_t column);
int send_opponent_move_pkt(SOCKET socket, maze_size_t row, maze_size_t column);
int send_bad_move_pkt(SOCKET socket, maze_size_t last_row, maze_size_t last_column);

int send_timeout_pkt(SOCKET socket) {
    mrmp_pkt_header_t msg = {
        .opcode = MRMP_OPCODE_TIMEOUT,
        .length = 0
    };

    int total_bytes_sent = 0;
    int bytes_expected = sizeof(msg);

    while(total_bytes_sent < bytes_expected) {
        int bytes_sent = send(socket, (const char*) &msg + total_bytes_sent, bytes_expected - total_bytes_sent, 0);
        if(bytes_sent == SOCKET_ERROR) {
            fprintf(stderr, "failed to send timeout packet\n");
            return DISGRACEFUL_DC;
        }
        total_bytes_sent += bytes_sent;
    }

    return SUCCESS; 
}

int recv_w_timeout(SOCKET socket, char* buffer, int length, int flags, struct timeval* timeout) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(socket, &readfds);

    // Wait for socket to become readable
    int select_result = select(0, &readfds, NULL, NULL, timeout);

    if(select_result == SOCKET_ERROR) {
        return SOCKET_ERROR;
    } else if(select_result == 0) {
        return TIMEDOUT;
    }
        
    return recv(socket, buffer, length, flags);
}

int receive_mrmp_msg(SOCKET socket, char** out_msg, struct timeval* timeout) {
    //first try to read sizeof(mrmp_pkt_header) bytes to get the operation code.
    char* msg = malloc(sizeof(mrmp_pkt_header_t));

    int total_bytes_received = 0;
    int expected_bytes = sizeof(mrmp_pkt_header_t);

    while(total_bytes_received < expected_bytes) {
        int bytes_received = recv_w_timeout(socket, msg + total_bytes_received, expected_bytes - total_bytes_received, 0, timeout);
        //error handle
        if(bytes_received == 0) {
            fprintf(stderr, "failed to receive message header, other end disconnected gracefully.\n");
            free(msg);
            return GRACEFUL_DC;
        } else if(bytes_received == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET) {
            fprintf(stderr, "failed to receive message header, other end disconnected ungracefully.\n");
            free(msg);
            return DISGRACEFUL_DC;
        } else if(bytes_received == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT) {
            fprintf(stderr, "failed to receive message header, other end timed out.\n");
            free(msg);
            return TIMEDOUT;
        }

        total_bytes_received += bytes_received;
    }

    //reset variables.
    expected_bytes += ((mrmp_pkt_header_t*)msg)->length;
    printf("%d byte message\n", ((mrmp_pkt_header_t*)msg)->length);

    //try to receive rest of message.
    msg = realloc(msg, expected_bytes);
    
    while(total_bytes_received < expected_bytes) {
        int bytes_received = recv_w_timeout(socket, msg + total_bytes_received, expected_bytes - total_bytes_received, 0, timeout);

        //error handle
        if(bytes_received == 0) {
            fprintf(stderr, "failed to receive message payload, other end disconnected gracefully.\n");
            free(msg);
            return GRACEFUL_DC;
        } else if(bytes_received == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET) {
            fprintf(stderr, "failed to receive message payload, other end disconnected ungracefully.\n");
            free(msg);
            return DISGRACEFUL_DC;
        } else if(bytes_received == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT) {
            fprintf(stderr, "failed to receive message payload, other end timed out.\n");
            free(msg);
            return TIMEDOUT;
        }

        total_bytes_received += bytes_received;
    }

    *out_msg = msg;

    return SUCCESS;
}

// char* buffer_to_mrmp_pkt_struct(char* buffer) {
//     mrmp_opcode_t opcode = (mrmp_opcode_t) buffer[0];
//     mrmp_pkt_header_t header = {
//         .opcode = opcode,
//         .length = *((mrmp_payload_size_t*)(buffer + sizeof(mrmp_opcode_t)))
//     };

//     char* pkt = NULL;

//     switch(opcode) {
//         case MRMP_OPCODE_HELLO_ACK:
//         case MRMP_OPCODE_JOIN:
//         case MRMP_OPCODE_LEAVE:
//         case MRMP_OPCODE_TIMEOUT:
//             pkt = malloc(sizeof(mrmp_pkt_header_t));
//             *((mrmp_pkt_header_t*)pkt) = header;
//             break;
//         case MRMP_OPCODE_ERROR:
//             pkt = malloc(sizeof(mrmp_pkt_error_t));
//             ((mrmp_pkt_error_t*)pkt)->header = header;
//             ((mrmp_pkt_error_t*)pkt)->error_code = buffer[MRMP_PKT_HEADER_SIZE];
//             break;
//         case MRMP_OPCODE_HELLO:
//             pkt = malloc(sizeof(mrmp_pkt_hello_t));
//             ((mrmp_pkt_hello_t*)pkt)->header = header;
//             ((mrmp_pkt_hello_t*)pkt)->version = buffer[MRMP_PKT_HEADER_SIZE];
//             break;
//         case MRMP_OPCODE_MOVE:
//             pkt = malloc(sizeof(mrmp_pkt_move_t));
//             ((mrmp_pkt_move_t*)pkt)->header = header;
//             ((mrmp_pkt_move_t*)pkt)->row = buffer[MRMP_PKT_HEADER_SIZE];
//             ((mrmp_pkt_move_t*)pkt)->column = buffer[MRMP_PKT_HEADER_SIZE + sizeof(maze_size_t)];
//             break;
//         case MRMP_OPCODE_BAD_MOVE:
//             pkt = malloc(sizeof(mrmp_pkt_move_t));
//             ((mrmp_pkt_move_t*)pkt)->header = header;
//             ((mrmp_pkt_move_t*)pkt)->row = buffer[MRMP_PKT_HEADER_SIZE];
//             ((mrmp_pkt_move_t*)pkt)->column = buffer[MRMP_PKT_HEADER_SIZE + sizeof(maze_size_t)];
//             break;
//         case MRMP_OPCODE_OPPONENT_MOVE:
//             pkt = malloc(sizeof(mrmp_pkt_move_t));
//             ((mrmp_pkt_move_t*)pkt)->header = header;
//             ((mrmp_pkt_move_t*)pkt)->row = buffer[MRMP_PKT_HEADER_SIZE];
//             ((mrmp_pkt_move_t*)pkt)->column = buffer[MRMP_PKT_HEADER_SIZE + sizeof(maze_size_t) * 2];
//             break;
//         default:
//             break;
//     }
// }

// int send_error_pkt(SOCKET socket, mrmp_error_t error) {
//     mrmp_pkt_error_t msg = {
//         {
//             .opcode = MRMP_OPCODE_ERROR,
//             .length = htonl(sizeof(mrmp_error_t))
//         },
//         .error_code = error
//     };

//     int field_address = 0;
 
//     char* buffer = malloc(MRMP_PKT_ERROR_SIZE);
//     memcpy(buffer, &msg.header.opcode, sizeof(mrmp_opcode_t));
//     field_address += sizeof(mrmp_opcode_t);
//     memcpy(buffer + field_address, &msg.header.length, sizeof(mrmp_payload_size_t));
//     field_address += sizeof(mrmp_payload_size_t);
//     memcpy(buffer + field_address, &msg.error_code, sizeof(mrmp_error_t));
//     field_address += sizeof(mrmp_error_t);

//     int send_buffer_result = send_buffer(socket, buffer, MRMP_PKT_ERROR_SIZE);

//     if(send_buffer_result == SOCKET_ERROR) {
//         fprintf(stderr, "failed to send error packet.\n");
//     }

//     free(buffer);

//     return send_buffer_result;
// }

// int send_hello_pkt(SOCKET socket, mrmp_version_t version) {
//     mrmp_pkt_hello_t msg = {
//         {
//             .opcode = MRMP_OPCODE_HELLO,
//             .length = sizeof(mrmp_version_t)
//         },
//         .version = version
//     };

//     msg.header.length = htonl(msg.header.length);

//     printf("%d", msg.header.length);
//     int field_address = 0;
 
//     char* buffer = malloc(MRMP_PKT_HELLO_SIZE);
//     memcpy(buffer, &msg.header.opcode, sizeof(mrmp_opcode_t));
//     field_address += sizeof(mrmp_opcode_t);
//     memcpy(buffer + field_address, &msg.header.length, sizeof(mrmp_payload_size_t));
//     field_address += sizeof(mrmp_payload_size_t);
//     memcpy(buffer + field_address, &msg.version, sizeof(mrmp_version_t));
//     field_address += sizeof(mrmp_version_t);

//     int send_buffer_result = send_buffer(socket, buffer, MRMP_PKT_HELLO_SIZE);

//     if(send_buffer_result == SOCKET_ERROR) {
//         fprintf(stderr, "failed to send hello packet.\n");
//     }

//     free(buffer);

//     return send_buffer_result;
// }

// int send_hello_ack_pkt(SOCKET socket) {
//     mrmp_pkt_header_t msg = {
//         .opcode = MRMP_OPCODE_HELLO_ACK,
//         .length = 0
//     };

//     int field_address = 0;
 
//     char* buffer = malloc(MRMP_PKT_HEADER_SIZE);
//     memcpy(buffer, &msg.opcode, sizeof(mrmp_opcode_t));
//     field_address += sizeof(mrmp_opcode_t);
//     memcpy(buffer + field_address, &msg.length, sizeof(mrmp_payload_size_t));
//     field_address += sizeof(mrmp_payload_size_t);

//     int send_buffer_result = send_buffer(socket, buffer, MRMP_PKT_HEADER_SIZE);

//     if(send_buffer_result == SOCKET_ERROR) {
//         fprintf(stderr, "failed to send error packet.\n");
//     }

//     free(buffer);

//     return send_buffer_result;
// }

// int send_join_pkt(SOCKET socket) {
//     mrmp_pkt_header_t msg = {
//         .opcode = MRMP_OPCODE_JOIN,
//         .length = 0
//     };

//     int field_address = 0;
 
//     char* buffer = malloc(MRMP_PKT_HEADER_SIZE);
//     memcpy(buffer, &msg.opcode, sizeof(mrmp_opcode_t));
//     field_address += sizeof(mrmp_opcode_t);
//     memcpy(buffer + field_address, &msg.length, sizeof(mrmp_payload_size_t));
//     field_address += sizeof(mrmp_payload_size_t);

//     int send_buffer_result = send_buffer(socket, buffer, MRMP_PKT_HEADER_SIZE);

//     if(send_buffer_result == SOCKET_ERROR) {
//         fprintf(stderr, "failed to send error packet.\n");
//     }

//     free(buffer);

//     return send_buffer_result;
// }

// int send_join_resp_pkt(SOCKET socket, maze_t* maze);
// int send_leave_pkt(SOCKET socket);
// int send_move_pkt(SOCKET socket, maze_size_t row, maze_size_t column);
// int send_opponent_move_pkt(SOCKET socket, maze_size_t row, maze_size_t column);
// int send_bad_move_pkt(SOCKET socket, maze_size_t last_row, maze_size_t last_column);

// int send_timeout_pkt(SOCKET socket) {
//     mrmp_pkt_header_t msg = {
//         .opcode = MRMP_OPCODE_TIMEOUT,
//         .length = 0
//     };

//     int total_bytes_sent = 0;
//     int bytes_expected = sizeof(msg);

//     while(total_bytes_sent < bytes_expected) {
//         int bytes_sent = send(socket, (const char*) &msg + total_bytes_sent, bytes_expected - total_bytes_sent, 0);
//         if(bytes_sent == SOCKET_ERROR) {
//             fprintf(stderr, "failed to send timeout packet\n");
//             return DISGRACEFUL_DC;
//         }
//         total_bytes_sent += bytes_sent;
//     }

//     return SUCCESS; 
// }

// int recv_w_timeout(SOCKET socket, char* buffer, int length, int flags, struct timeval* timeout) {
//     fd_set readfds;
//     FD_ZERO(&readfds);
//     FD_SET(socket, &readfds);

//     // Wait for socket to become readable
//     int select_result = select(0, &readfds, NULL, NULL, timeout);

//     if(select_result == SOCKET_ERROR) {
//         return SOCKET_ERROR;
//     } else if(select_result == 0) {
//         return TIMEDOUT;
//     }
        
//     return recv(socket, buffer, length, flags);
// }

// int receive_mrmp_msg(SOCKET socket, char** out_msg, struct timeval* timeout) {
//     //first try to read enough bytes for the fields in mrmp_pkt_header_t to get the operation code.
//     char* bytes = malloc(MRMP_PKT_HEADER_SIZE);

//     int total_bytes_received = 0;
//     int expected_bytes = MRMP_PKT_HEADER_SIZE;

//     while(total_bytes_received < expected_bytes) {
//         int bytes_received = recv_w_timeout(socket, bytes + total_bytes_received, expected_bytes - total_bytes_received, 0, timeout);
//         //error handle
//         if(bytes_received == 0) {
//             fprintf(stderr, "failed to receive message header, other end disconnected gracefully.\n");
//             free(bytes);
//             return GRACEFUL_DC;
//         } else if(bytes_received == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET) {
//             fprintf(stderr, "failed to receive message header, other end disconnected ungracefully.\n");
//             free(bytes);
//             return DISGRACEFUL_DC;
//         } else if(bytes_received == TIMEDOUT) {
//             fprintf(stderr, "failed to receive message header, other end timed out.\n");
//             free(bytes);
//             return TIMEDOUT;
//         }

//         total_bytes_received += bytes_received;
//     }
    
//     mrmp_payload_size_t payload_length = *((mrmp_payload_size_t*)(bytes + sizeof(mrmp_opcode_t)));
//     payload_length = ntohl(payload_length);
//     *((mrmp_payload_size_t*)(bytes + sizeof(mrmp_opcode_t))) = payload_length;

//     //reset variables.
//     expected_bytes += payload_length; //((mrmp_pkt_header_t*)bytes)->length;

//     fprintf(stderr, "%d byte message received\n", payload_length);

//     //try to receive rest of message.
//     bytes = realloc(bytes, expected_bytes);
    
//     while(total_bytes_received < expected_bytes) {
//         int bytes_received = recv_w_timeout(socket, bytes + total_bytes_received, expected_bytes - total_bytes_received, 0, timeout);

//         //error handle
//         if(bytes_received == 0) {
//             fprintf(stderr, "failed to receive message payload, other end disconnected gracefully.\n");
//             free(bytes);
//             return GRACEFUL_DC;
//         } else if(bytes_received == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET) {
//             fprintf(stderr, "failed to receive message payload, other end disconnected ungracefully.\n");
//             free(bytes);
//             return DISGRACEFUL_DC;
//         } else if(bytes_received == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT) {
//             fprintf(stderr, "failed to receive message payload, other end timed out.\n");
//             free(bytes);
//             return TIMEDOUT;
//         }

//         total_bytes_received += bytes_received;
//     }

//     *out_msg = buffer_to_mrmp_pkt_struct(bytes);

//     return SUCCESS;
// }



