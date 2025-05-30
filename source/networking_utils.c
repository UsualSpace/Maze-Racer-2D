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

int send_buffer(SOCKET socket, const char* buffer, int buffer_length) {
    int total_bytes_sent = 0;
    int bytes_expected = buffer_length;

    while(total_bytes_sent < bytes_expected) {
        int bytes_sent = send(socket, buffer + total_bytes_sent, bytes_expected - total_bytes_sent, 0);
        if(bytes_sent == SOCKET_ERROR) {
            return SOCKET_ERROR;
        }
        total_bytes_sent += bytes_sent;
    }

    return SUCCESS;
}

char* buffer_to_mrmp_pkt_struct(char* buffer) {
    mrmp_pkt_header_t header;
    memcpy(&header.opcode, buffer, sizeof(mrmp_opcode_t));
    memcpy(&header.length, buffer + sizeof(mrmp_opcode_t), sizeof(mrmp_payload_size_t));

    header.length = ntohl(header.length);

    char* pkt = NULL;

    switch(header.opcode) {
        case MRMP_OPCODE_HELLO_ACK:
        case MRMP_OPCODE_JOIN:
        case MRMP_OPCODE_LEAVE:
        case MRMP_OPCODE_TIMEOUT:
            pkt = malloc(sizeof(mrmp_pkt_header_t));
            memcpy(pkt, &header, sizeof(mrmp_pkt_header_t));
            break;
        case MRMP_OPCODE_ERROR:
            pkt = malloc(sizeof(mrmp_pkt_error_t));
            memcpy(pkt, &header, sizeof(mrmp_pkt_header_t));
            memcpy(&((mrmp_pkt_error_t*)pkt)->error_code, buffer + MRMP_PKT_HEADER_SIZE, sizeof(mrmp_error_t));
            break;
        case MRMP_OPCODE_HELLO:
            pkt = malloc(sizeof(mrmp_pkt_hello_t));
            memcpy(pkt, &header, sizeof(mrmp_pkt_header_t));
            memcpy(&((mrmp_pkt_hello_t*)pkt)->version, buffer + MRMP_PKT_HEADER_SIZE, sizeof(mrmp_version_t));
            break;
        case MRMP_OPCODE_MOVE:
        case MRMP_OPCODE_BAD_MOVE:
        case MRMP_OPCODE_OPPONENT_MOVE:
            pkt = malloc(sizeof(mrmp_pkt_move_t));
            memcpy(pkt, &header, sizeof(mrmp_pkt_header_t));
            memcpy(&((mrmp_pkt_move_t*)pkt)->row, buffer + MRMP_PKT_HEADER_SIZE, sizeof(maze_size_t));
            memcpy(&((mrmp_pkt_move_t*)pkt)->column, buffer + MRMP_PKT_HEADER_SIZE + sizeof(maze_size_t), sizeof(maze_size_t));
            break;
        case MRMP_OPCODE_JOIN_RESP:
            {
                maze_size_t rows, columns;
                memcpy(&rows, buffer + MRMP_PKT_HEADER_SIZE, sizeof(maze_size_t));
                memcpy(&columns, buffer + MRMP_PKT_HEADER_SIZE + sizeof(maze_size_t), sizeof(maze_size_t));
                pkt = malloc(sizeof(mrmp_pkt_join_resp_t) + (rows * columns) * sizeof(maze_cell_t));
                memcpy(pkt, &header, sizeof(mrmp_pkt_header_t));
                memcpy(&((mrmp_pkt_join_resp_t*)pkt)->rows, buffer + MRMP_PKT_HEADER_SIZE, sizeof(maze_size_t));
                memcpy(&((mrmp_pkt_join_resp_t*)pkt)->columns, buffer + MRMP_PKT_HEADER_SIZE + sizeof(maze_size_t), sizeof(maze_size_t));
                memcpy(((mrmp_pkt_join_resp_t*)pkt)->cells, buffer + MRMP_PKT_HEADER_SIZE + sizeof(maze_size_t) * 2, (rows * columns) * sizeof(maze_cell_t));
            }
            break;
        default:
            break;
    }

    return pkt;
}

int send_error_pkt(SOCKET socket, mrmp_error_t error) {
    mrmp_pkt_error_t msg = {
        {
            .opcode = MRMP_OPCODE_ERROR,
            .length = sizeof(mrmp_error_t)
        },
        .error_code = error
    };

    msg.header.length = htonl(msg.header.length);

    int field_address = 0;
 
    char* buffer = malloc(MRMP_PKT_ERROR_SIZE);
    memcpy(buffer, &msg.header.opcode, sizeof(mrmp_opcode_t));
    field_address += sizeof(mrmp_opcode_t);
    memcpy(buffer + field_address, &msg.header.length, sizeof(mrmp_payload_size_t));
    field_address += sizeof(mrmp_payload_size_t);
    memcpy(buffer + field_address, &msg.error_code, sizeof(mrmp_error_t));

    int send_buffer_result = send_buffer(socket, buffer, MRMP_PKT_ERROR_SIZE);

    if(send_buffer_result == SOCKET_ERROR) {
        fprintf(stderr, "failed to send error packet.\n");
    }

    free(buffer);

    return send_buffer_result;
}

int send_hello_pkt(SOCKET socket, mrmp_version_t version) {
    mrmp_pkt_hello_t msg = {
        {
            .opcode = MRMP_OPCODE_HELLO,
            .length = sizeof(mrmp_version_t)
        },
        .version = version
    };

    msg.header.length = htonl(msg.header.length);

    int field_address = 0;
 
    char* buffer = malloc(MRMP_PKT_HELLO_SIZE);
    memcpy(buffer, &msg.header.opcode, sizeof(mrmp_opcode_t));
    field_address += sizeof(mrmp_opcode_t);
    memcpy(buffer + field_address, &msg.header.length, sizeof(mrmp_payload_size_t));
    field_address += sizeof(mrmp_payload_size_t);
    memcpy(buffer + field_address, &msg.version, sizeof(mrmp_version_t));

    int send_buffer_result = send_buffer(socket, buffer, MRMP_PKT_HELLO_SIZE);

    if(send_buffer_result == SOCKET_ERROR) {
        fprintf(stderr, "failed to send hello packet.\n");
    }

    free(buffer);

    return send_buffer_result;
}

int send_hello_ack_pkt(SOCKET socket) {
    mrmp_pkt_header_t msg = {
        .opcode = MRMP_OPCODE_HELLO_ACK,
        .length = 0
    };

    int field_address = 0;
 
    char* buffer = malloc(MRMP_PKT_HEADER_SIZE);
    memcpy(buffer, &msg.opcode, sizeof(mrmp_opcode_t));
    field_address += sizeof(mrmp_opcode_t);
    memcpy(buffer + field_address, &msg.length, sizeof(mrmp_payload_size_t));

    printf("sending hello pkt ack\n");
    int send_buffer_result = send_buffer(socket, buffer, MRMP_PKT_HEADER_SIZE);

    if(send_buffer_result == SOCKET_ERROR) {
        fprintf(stderr, "failed to send error packet.\n");
    }

    free(buffer);

    return send_buffer_result;
}

int send_join_pkt(SOCKET socket) {
    mrmp_pkt_header_t msg = {
        .opcode = MRMP_OPCODE_JOIN,
        .length = 0
    };

    int field_address = 0;
 
    char* buffer = malloc(MRMP_PKT_HEADER_SIZE);
    memcpy(buffer, &msg.opcode, sizeof(mrmp_opcode_t));
    field_address += sizeof(mrmp_opcode_t);
    memcpy(buffer + field_address, &msg.length, sizeof(mrmp_payload_size_t));
    field_address += sizeof(mrmp_payload_size_t);

    int send_buffer_result = send_buffer(socket, buffer, MRMP_PKT_HEADER_SIZE);

    if(send_buffer_result == SOCKET_ERROR) {
        fprintf(stderr, "failed to send error packet.\n");
    }

    free(buffer);

    return send_buffer_result;
}

int send_join_resp_pkt(SOCKET socket, maze_t* maze) {
    mrmp_pkt_header_t msg = {
        .opcode = MRMP_OPCODE_JOIN_RESP,
        .length = sizeof(maze_size_t) * 2 + sizeof(maze_cell_t) * (maze->rows * maze->columns) 
    };

    int field_address = 0;
    int host_length = msg.length;
    msg.length = htonl(msg.length);

    char* buffer = malloc(MRMP_PKT_HEADER_SIZE + host_length);
    memcpy(buffer, &msg.opcode, sizeof(mrmp_opcode_t));
    field_address += sizeof(mrmp_opcode_t);
    memcpy(buffer + field_address, &msg.length, sizeof(mrmp_payload_size_t));
    field_address += sizeof(mrmp_payload_size_t);
    memcpy(buffer + field_address, &maze->rows, sizeof(maze_size_t));
    field_address += sizeof(maze_size_t);
    memcpy(buffer + field_address, &maze->columns, sizeof(maze_size_t));
    field_address += sizeof(maze_size_t);

    //copy maze cells.
    for(maze_size_t row = 0; row < maze->rows; ++row) {
        memcpy(buffer + field_address, &maze->cells[row], sizeof(maze_cell_t) * maze->columns);
        field_address += sizeof(maze_cell_t) * maze->columns;
    }

    int send_buffer_result = send_buffer(socket, buffer, MRMP_PKT_HEADER_SIZE + host_length);

    if(send_buffer_result == SOCKET_ERROR) {
        fprintf(stderr, "failed to send join resp packet.\n");
    }

    free(buffer);

    return send_buffer_result;
}

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

maze_t* maze_network_to_host(mrmp_pkt_join_resp_t* msg) {
    if(msg == NULL) {
        printf("mntoh returned null\n");
        return NULL;
    }
    maze_t* maze = malloc(sizeof(maze_t));
    maze->cells = malloc(sizeof(maze_cell_t*) * msg->rows);

    printf("mntoh rows: %d\n", msg->rows);

    maze->rows = msg->rows;
    maze->columns = msg->columns;

    for(maze_size_t row = 0; row < maze->rows; ++row) {
        maze->cells[row] = malloc(sizeof(maze_cell_t) * maze->columns);
    }

    //reshape the flattened 2D maze structure back into 2 dimensions.
    int i = 0;
    for(maze_size_t row = 0; row < maze->rows; ++row) {
        for(maze_size_t column = 0; column < maze->columns; ++column) {
            maze->cells[row][column] = msg->cells[i];
        }
    }

    return maze;
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
    //first try to read enough bytes for the fields in mrmp_pkt_header_t to get the operation code.
    char* bytes = malloc(MRMP_PKT_HEADER_SIZE);

    int total_bytes_received = 0;
    int expected_bytes = MRMP_PKT_HEADER_SIZE;

    while(total_bytes_received < expected_bytes) {
        int bytes_received = recv_w_timeout(socket, bytes + total_bytes_received, expected_bytes - total_bytes_received, 0, timeout);
        //error handle
        if(bytes_received == 0) {
            fprintf(stderr, "failed to receive message header, other end disconnected gracefully.\n");
            free(bytes);
            return GRACEFUL_DC;
        } else if(bytes_received == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET) {
            fprintf(stderr, "failed to receive message header, other end disconnected ungracefully.\n");
            free(bytes);
            return DISGRACEFUL_DC;
        } else if(bytes_received == TIMEDOUT) {
            fprintf(stderr, "failed to receive message header, other end timed out.\n");
            free(bytes);
            return TIMEDOUT;
        }

        total_bytes_received += bytes_received;
    }
    
    mrmp_payload_size_t payload_length;
    memcpy(&payload_length, bytes + sizeof(mrmp_opcode_t), sizeof(mrmp_payload_size_t));
    payload_length = ntohl(payload_length);

    //reset variables.
    expected_bytes += payload_length; //((mrmp_pkt_header_t*)bytes)->length;

    fprintf(stderr, "%d byte message received\n", payload_length);

    //try to receive rest of message.
    bytes = realloc(bytes, expected_bytes);
    
    while(total_bytes_received < expected_bytes) {
        int bytes_received = recv_w_timeout(socket, bytes + total_bytes_received, expected_bytes - total_bytes_received, 0, timeout);

        //error handle
        if(bytes_received == 0) {
            fprintf(stderr, "failed to receive message payload, other end disconnected gracefully.\n");
            free(bytes);
            return GRACEFUL_DC;
        } else if(bytes_received == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET) {
            fprintf(stderr, "failed to receive message payload, other end disconnected ungracefully.\n");
            free(bytes);
            return DISGRACEFUL_DC;
        } else if(bytes_received == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT) {
            fprintf(stderr, "failed to receive message payload, other end timed out.\n");
            free(bytes);
            return TIMEDOUT;
        }

        total_bytes_received += bytes_received;
    }

    printf("reached\n");
    *out_msg = buffer_to_mrmp_pkt_struct(bytes);
    free(bytes);

    return SUCCESS;
}



