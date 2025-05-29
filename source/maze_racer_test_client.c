// Filename: maze_racer_test_client.c
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/27/2025
// Purpose: To confirm messages are being properly sent back and forth.

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "networking_utils.h"
#include "maze.h"

#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS 0
#endif //EXIT_SUCCESS
#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif //EXIT_FAILURE

int main(int argc, char* argv[]) {

    //initialize winsock.
    WSADATA wsa_data;
    int wsa_startup_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if(wsa_startup_result != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", wsa_startup_result);
        return EXIT_FAILURE;
    }

    struct addrinfo *result = NULL, *ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int getaddrinfo_result = getaddrinfo(argv[1], MRMP_DEFAULT_PORT, &hints, &result);
    if(getaddrinfo_result != 0) {
        fprintf(stderr, "getaddrinfo failed: %d\n", getaddrinfo_result);
        WSACleanup();
        return EXIT_FAILURE;
    }

    //connecting socket setup.
    SOCKET connect_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(connect_socket == INVALID_SOCKET) {
        fprintf(stderr, "error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    //connect to server
    int connect_result = connect(connect_socket, result->ai_addr, (int)result->ai_addrlen);
    if (connect_result == SOCKET_ERROR) {
        closesocket(connect_socket);
        connect_socket = INVALID_SOCKET;
    }

    // Should really try the next address returned by getaddrinfo
    // if the connect call failed
    // But for this simple example we just free the resources
    // returned by getaddrinfo and print an error message

    freeaddrinfo(result);

    if (connect_result == INVALID_SOCKET) {
        fprintf(stderr, "unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    int hello_result = send_hello_pkt(connect_socket, 0);

    void* msg = NULL;
    receive_mrmp_msg(connect_socket, &msg);
    // if(((mrmp_pkt_error_t*) msg)->error_code == MRMP_ERR_VERSION_MISMATCH) {
    //     printf("Received version mismatch error packet!\n");
    // } else {
    //     //printf("Did NOT receive version mismatch error packet :(\n");
    // }

    if(((mrmp_pkt_header_t*) msg)->opcode == MRMP_OPCODE_HELLO_ACK) {
        printf("Received hello acknowledgement packet!\n");
    } else {
        //printf("Did NOT receive hello acknowledgement packet :(\n");
    }


    Sleep(12 * 1000);

    closesocket(connect_socket);

    return EXIT_SUCCESS;
}