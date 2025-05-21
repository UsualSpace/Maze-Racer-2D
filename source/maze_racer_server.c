// Filename: maze_racer_server.h
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/19/2025
// Purpose: To implement the server side of the application.

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <SDL_thread.h>

#include "player_queue.h"

#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS 0
#endif //EXIT_SUCCESS
#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif //EXIT_FAILURE

#ifndef TRUE
# define TRUE 1
#endif //TRUE
#ifndef FALSE
# define FALSE 0
#endif //FALSE

//defines
#define MAX_SESSION_THREADS     10
#define MAX_CLIENT_CONNECTIONS (MAX_SESSION_THREADS * 2)
#define MAX_TIMEOUT_SECONDS     10
#define DEFAULT_LISTEN_PORT     "9898"

#define CMD_EXIT    "exit"
#define CMD_STAT    "stat"
#define CMD_HELP    "help"
#define CMD_PPV     "++v"
#define CMD_MMV     "--v"
#define CMD_MAX_LEN 4

//help message for server ui
const char* SERVER_UI_WELCOME = "Welcome to the MRMP server interface!";
const char* SERVER_UI_HELP =    "=================================================\n"
                                "Below are a list of available commands:\n"
                                "=================================================\n"
                                "stat : Display the # of total and active\n" 
                                "       connections and sessions.\n"
                                "help : Display this very same help message.\n"
                                "++v  : Enable verbosity.\n"
                                "--v  : Disable verbosity.\n"
                                "exit : Exit the server process, shutting down everything.\n";

//stores all ready players
static player_queue_t* queue = NULL;
static SOCKET listen_socket = INVALID_SOCKET;

//for statistical/debug purposes.
static int total_connections = 0;
static int active_connections = 0;
static int total_sessions = 0;
static int active_sessions = 0;
static int verbose = FALSE;

//functions
void cleanup(void);
int server_ui(void* data);
int client_limbo(void* data);
int do_session(void* session_state);

int main(void) {
    //register functions to be called at exit().
    atexit(cleanup);

    //initialize SDL
    if(SDL_Init(0) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    //start up minimal user interface thread.
    SDL_Thread* server_ui_thread = SDL_CreateThread(server_ui, "MRMP Server UI Thread", server_ui_thread);

    //initialize player queue.
    queue = player_queue_init();

    //initialize winsock.
    WSADATA wsa_data;
    int wsa_startup_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if(wsa_startup_result != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", wsa_startup_result);
        return EXIT_FAILURE;
    }

    struct addrinfo* result = NULL, *ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int getaddrinfo_result = getaddrinfo(NULL, DEFAULT_LISTEN_PORT, &hints, &result);
    if(getaddrinfo_result != 0) {
        fprintf(stderr, "getaddrinfo failed: %d\n", getaddrinfo_result);
        WSACleanup();
        return EXIT_FAILURE;
    }

    //listening socket setup.
    listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(listen_socket == INVALID_SOCKET) {
        fprintf(stderr, "error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    int bind_result = bind(listen_socket, result->ai_addr, (int)result->ai_addrlen);
    if(bind_result == SOCKET_ERROR) {
        fprintf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listen_socket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    //not needed anymore
    freeaddrinfo(result);

    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "listen failed with error: %ld\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    //client connection listen loop.
    while(1) {
        SOCKET client_socket = accept(listen_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            fprintf(stderr, "accept failed: %d\n", WSAGetLastError());
            continue;
        }

        //create a new thread to house client connection.
    }

    return EXIT_SUCCESS;
}

void cleanup(void) {
    closesocket(listen_socket);
    player_queue_free(queue);
}

int server_ui(void* data) {
    //detach this thread so we dont have to worry about cleaning it up.
    SDL_DetachThread((SDL_Thread*) data);

    char cmd_buffer[CMD_MAX_LEN + 1];

    //introduction.
    printf("%s\n%s\n", SERVER_UI_WELCOME, SERVER_UI_HELP);

    //styling.
    printf(">> ");
    //setup simple terminal.
    while(fgets(cmd_buffer, CMD_MAX_LEN, stdin) != NULL) {
        if(strncmp(cmd_buffer, CMD_EXIT, 4) == 0) {
            printf("Server process is exiting...\n");
            break;
        } else if(strncmp(cmd_buffer, CMD_STAT, 4) == 0) {

        } else if(strncmp(cmd_buffer, CMD_HELP, 4) == 0) {
            printf("%s", SERVER_UI_HELP);
        } else if(strncmp(cmd_buffer, CMD_PPV, 3) == 0) {
            if(verbose == TRUE) {
                printf("Verbose mode is enabled.\n");
            } else {
                verbose = TRUE;
                printf("Enabled verbose mode.\n");
            }
        } else if(strncmp(cmd_buffer, CMD_MMV, 3) == 0) {
            if(verbose == FALSE) {
                printf("Verbose mode is disabled.\n");
            } else {
                verbose = TRUE;
                printf("Disabled verbose mode.\n");
            }
        } else {
            printf("Unknown command received.\n");
        }
        printf(">> ");
    }

    exit(EXIT_SUCCESS);
}