// Filename: maze_racer_server.h
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/19/2025
// Purpose: To implement the server side of the application.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "player_queue.h"
#include "networking_utils.h"

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
#define MAX_SESSION_THREADS         10
#define MAX_CLIENT_CONNECTIONS      (MAX_SESSION_THREADS * 2)
#define DEFAULT_TIMEOUT_SECONDS     1
#define MRMP_VERSION                0

#define CMD_EXIT    "exit"
#define CMD_STAT    "stat"
#define CMD_HELP    "help"
#define CMD_PQUE    "pque"
#define CMD_PPV     "++v"
#define CMD_MMV     "--v"
#define CMD_MAX_LEN 4

//help message for server ui
const char* SERVER_UI_WELCOME = "Welcome to the MRMP server interface!";
const char* SERVER_UI_HELP =    "Below are a list of available commands:\n"
                                "\tstat : Display the # of total and active\n" 
                                "\t       connections and sessions.\n"
                                "\tpque : Display the # of clients waiting in the player queue.\n"
                                "\thelp : Display this very same help message.\n"
                                "\t++v  : Enable verbosity.\n"
                                "\t--v  : Disable verbosity.\n"
                                "\texit : Exit the server process, shutting down everything.\n";

//for client connection/ready player tracking purposes.
static player_queue_t* player_queue = NULL;
static SOCKET listen_socket = INVALID_SOCKET;
static SOCKET accepted_sockets[MAX_CLIENT_CONNECTIONS]; //for global graceful disconnection handling.
static HANDLE session_threads[MAX_SESSION_THREADS];

//stores the next available cell in the client_sockets array to store an accepted client connection.
static int next_thread_idx = 0;

//for statistical/debug purposes.
static int total_connections = 0;
static volatile int active_connections = 0; //needs concurrency.
static int total_sessions = 0;
static volatile int active_sessions = 0; //needs concurrency.

//other server specific variables.
static int verbose = FALSE;
static volatile int quit = FALSE;
static HANDLE server_ui_thread;
static HANDLE create_sessions_thread;
static CRITICAL_SECTION player_queue_critsec;
static CRITICAL_SECTION server_state_critsec;

static struct timeval DEFAULT_TIMEOUT = {
    .tv_sec = DEFAULT_TIMEOUT_SECONDS,
    .tv_usec = 0
};

//functions
void cleanup(void);

unsigned __stdcall server_ui(void* data);
unsigned __stdcall client_limbo(void* data);
unsigned __stdcall do_session(void* session_state);
unsigned __stdcall create_sessions(void* data);

int main(void) {
    //register functions to be called at exit().
    atexit(cleanup);

    InitializeCriticalSection(&player_queue_critsec);
    InitializeCriticalSection(&server_state_critsec);

    //start up minimal user interface thread.
    server_ui_thread = (HANDLE)_beginthreadex(NULL, 0, &server_ui, NULL, 0, NULL);
    if(server_ui_thread == NULL) {
        fprintf(stderr, "failed to create user interface thread.\n");
        return EXIT_FAILURE;
    }

    //initialize player queue.
    player_queue = player_queue_init();

    //start up session creation thread.
    create_sessions_thread = (HANDLE)_beginthreadex(NULL, 0, &create_sessions, NULL, 0, NULL);
    if(create_sessions_thread == NULL) {
        fprintf(stderr, "failed to create create sessions thread.\n");
        return EXIT_FAILURE;
    }

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

    int getaddrinfo_result = getaddrinfo(NULL, MRMP_DEFAULT_PORT, &hints, &result);
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
        fprintf(stderr, "bind() failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listen_socket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    //not needed anymore
    freeaddrinfo(result);

    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "listen() failed with error: %ld\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    //make listen socket non-blocking for accept.
    u_long mode = 1; 
    int ioctl_result = ioctlsocket(listen_socket, FIONBIO, &mode);
    if(ioctl_result != NO_ERROR) {
        fprintf(stderr, "ioctlsocket failed with error: %ld\n", ioctl_result);
    }

    //client connection listen loop.
    while(quit != TRUE) {
        if(active_connections > MAX_CLIENT_CONNECTIONS) continue;
        
        SOCKET* client_socket = malloc(sizeof(SOCKET));
        if(client_socket == NULL) {
            fprintf(stderr, "failed to malloc() client socket pointer.\n");
            continue;
        }

        *client_socket = accept(listen_socket, NULL, NULL);
        if (*client_socket == INVALID_SOCKET) {
            //fprintf(stderr, "accept() failed: %d\n", WSAGetLastError());
            free(client_socket);
            continue;
        }

        //THIS CODE MAKES A SOCKET STALE, UNUSABLE.
        //set a timeout value for future recv operations.
        // DWORD timeout = DEFAULT_TIMEOUT_SECONDS * 1000;
        // int setsockopt_result = setsockopt(*client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(timeout));
        // if(setsockopt_result == SOCKET_ERROR) {
        //     fprintf(stderr, "setsockopt failed with %u\n", WSAGetLastError());
        //     send_error_pkt(*client_socket, MRMP_ERR_UNKNOWN);
        //     shutdown(*client_socket, SD_SEND);
        //     closesocket(*client_socket);
        //     free(client_socket);
        // }

        //create a new thread to house client connection.
        HANDLE limbo_thread = (HANDLE)_beginthreadex(NULL, 0, &client_limbo, client_socket, 0, NULL);
        if(limbo_thread == NULL) {
            fprintf(stderr, "failed to create client limbo thread.\n");
            send_error_pkt(*client_socket, MRMP_ERR_UNKNOWN);
            shutdown(*client_socket, SD_SEND);
            closesocket(*client_socket);
            free(client_socket);
        }

        CloseHandle(limbo_thread);
    }

    return EXIT_SUCCESS;
}

void cleanup(void) {
    closesocket(listen_socket);
    WSACleanup();

    WaitForSingleObject(create_sessions_thread, INFINITE);
    CloseHandle(create_sessions_thread);

    //TODO: make sure this is the right way to clean up a critical section.
    DeleteCriticalSection(&player_queue_critsec);
    DeleteCriticalSection(&server_state_critsec);

    player_queue_free(player_queue);

    WaitForSingleObject(server_ui_thread, INFINITE);
    CloseHandle(server_ui_thread);
}

unsigned __stdcall server_ui(void* data) {
    char cmd_buffer[CMD_MAX_LEN + 2]; //+2 for new line and null byte.

    //introduction.
    printf("%s\n%s\n", SERVER_UI_WELCOME, SERVER_UI_HELP);

    //styling.
    printf(">> ");
    //setup simple terminal.
    while(fgets(cmd_buffer, CMD_MAX_LEN + 2, stdin) != NULL) {
        ssize_t len = strlen(cmd_buffer);
        if(len > 0 && cmd_buffer[len - 1] != '\n') {
            int c;
            while(c = getchar() != '\n' && c != EOF); //clear stdin.
        }

        if(strncmp(cmd_buffer, CMD_EXIT, 4) == 0) {
            printf("Server process is exiting...\n");
            break;
        } else if(strncmp(cmd_buffer, CMD_STAT, 4) == 0) {

        } else if(strncmp(cmd_buffer, CMD_PQUE, 4) == 0) {
            printf("%d clients currently waiting in the player queue\n", player_queue_size(player_queue));
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
                verbose = FALSE;
                printf("Disabled verbose mode.\n");
            }
        } else {
            printf("Unknown command received.\n");
        }
        printf(">> ");
    }

    quit = TRUE;

    _endthreadex(0);
    return 0;
}

unsigned __stdcall client_limbo(void* client_socket) {
    SOCKET socket = *(SOCKET*) client_socket;
    
    //temporary fix to repetitive cleanup code.
    int exit_flag = FALSE;

    //try to receive a hello packet, if it's not received correctly or in time, send an error packet 
    //and or shutdown and close the socket.
    mrmp_pkt_hello_t* hello_msg = NULL;
    int hello_result = receive_mrmp_msg(socket, (char**) &hello_msg, &DEFAULT_TIMEOUT);
    
    switch(hello_result) {
        case GRACEFUL_DC:
            send_error_pkt(socket, MRMP_ERR_UNKNOWN); //TODO: do we need this?
            shutdown(socket, SD_SEND);
            exit_flag = TRUE;
            printf("GRACEFUL DC\n");
            break;
        case DISGRACEFUL_DC:
            exit_flag = TRUE;
            printf("DISGRACEFUL DC\n");
            break;
        case TIMEDOUT:
            send_timeout_pkt(socket);
            shutdown(socket, SD_SEND);
            exit_flag = TRUE;
            printf("TIMEDOUT\n");
            break;
        default:
            break;
    };

    if(!exit_flag && hello_msg->header.opcode != MRMP_OPCODE_HELLO) {
        send_error_pkt(socket, MRMP_ERR_ILLEGAL_OPCODE);
        shutdown(socket, SD_SEND);
        exit_flag = TRUE;
    } else if(!exit_flag && hello_msg->version != MRMP_VERSION) {
        if(verbose == TRUE)
            printf("Recieved hello packet, version %d\n", hello_msg->version);
        send_error_pkt(socket, MRMP_ERR_VERSION_MISMATCH);
        shutdown(socket, SD_SEND);
        exit_flag = TRUE;
        printf("wrong version\n");
    }

    if(exit_flag == TRUE) {
        closesocket(socket);
        free(hello_msg);
        free(client_socket);
        _endthreadex(0);
    }

    free(hello_msg);

    //succesful hello performed, send an app layer acknowledgment.
    send_hello_ack_pkt(socket);

    //exact same code but for the ready packet. TODO: find a better less repetitive way to handle this?
    mrmp_pkt_header_t* join_msg = NULL;
    int join_result = receive_mrmp_msg(socket, (char**) &join_msg, &DEFAULT_TIMEOUT);

    switch(join_result) {
        case GRACEFUL_DC:
            send_error_pkt(socket, MRMP_ERR_UNKNOWN); //TODO: do we need this?
            fprintf(stderr, "sent unknown error packet.\n");
            shutdown(socket, SD_SEND);
            exit_flag = TRUE;
            break;
        case DISGRACEFUL_DC:
            exit_flag = TRUE;
            break;
        case TIMEDOUT:
            send_timeout_pkt(socket);
            fprintf(stderr, "sent timeout packet.\n");
            shutdown(socket, SD_SEND);
            exit_flag = TRUE;
            break;
        default:
            break;
    };

    if(!exit_flag && join_msg->opcode != MRMP_OPCODE_JOIN) {
        send_error_pkt(socket, MRMP_ERR_ILLEGAL_OPCODE);
        fprintf(stderr, "sent illegal opcode error packet due to received opcode %d\n", join_msg->opcode);
        shutdown(socket, SD_SEND);
        exit_flag = TRUE;
    }

    if(exit_flag == TRUE) {
        closesocket(socket);
        free(join_msg);
        free(client_socket);
        _endthreadex(0);
    }

    free(join_msg);

    //successful join request, add them to the player queue and end this thread.
    EnterCriticalSection(&player_queue_critsec);
    player_queue_push(player_queue, socket);
    LeaveCriticalSection(&player_queue_critsec);

    shutdown(socket, SD_SEND);
    closesocket(socket);

    free(client_socket);
    _endthreadex(0);
    return 0;
}

unsigned __stdcall do_session(void* session_state) {
    session_t* session = session_state;

    _endthreadex(0);
    return 0;
}

unsigned __stdcall create_sessions(void* data) {
    //TODO: look into windows condition variables.
    while(quit != TRUE) {
        // EnterCriticalSection(&player_queue_critsec);
        // //check if there is at least 2 players waiting in queue.
        // //TODO: does reading active_sessions need to be locked?
        // while(player_queue_size(player_queue) >= 2 && active_sessions <= MAX_SESSION_THREADS) {
        //     //check validity of player sockets.
        //     SOCKET* player_one_sock = malloc(sizeof(SOCKET));
        //     SOCKET* player_two_sock = malloc(sizeof(SOCKET));

        //     *player_one_sock = *player_queue_front(player_queue);
        //     player_queue_pop(player_queue);
        //     *player_two_sock = *player_queue_front(player_queue);
        //     player_queue_pop(player_queue);
        // }

        // LeaveCriticalSection(&player_queue_critsec);
    }


    _endthreadex(0);
    return 0;
}