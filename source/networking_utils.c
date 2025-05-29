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

int recv_w_timeout(SOCKET socket, char* buffer, int length, int flags, DWORD timeout_secs) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(socket, &readfds);

    struct timeval timeout;
    timeout.tv_sec = timeout_secs;   // Wait up to 5 seconds
    timeout.tv_usec = 0;

    // Wait for socket to become readable
    int select_result = select(0, &readfds, NULL, NULL, &timeout);

    if(select_result == SOCKET_ERROR) {

    } else if(select_result == 0) {
        return 
    }
}

int receive_mrmp_msg(SOCKET socket, void** out_msg) {
    //first try to read sizeof(mrmp_pkt_header) bytes to get the operation code.
    void* msg = malloc(sizeof(mrmp_pkt_header_t));

    int total_bytes_received = 0;
    int expected_bytes = sizeof(mrmp_pkt_header_t);

    while(total_bytes_received < expected_bytes) {
        int bytes_received = recv(socket, (char*) (msg + total_bytes_received), expected_bytes - total_bytes_received, 0);
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
        int bytes_received = recv(socket, (char*) (msg + total_bytes_received), expected_bytes - total_bytes_received, 0);

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

