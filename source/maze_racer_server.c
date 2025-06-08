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
#define ACTIVITY_TIMEOUT_SECONDS    20
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

//for cleanup purposes.
typedef struct thread_tracker {
    HANDLE thread_handle;
    int is_active;
} thread_tracker_t;

static thread_tracker_t session_thread_tracker[MAX_SESSION_THREADS];

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

static struct timeval ACTIVITY_TIMEOUT = {
    .tv_sec = ACTIVITY_TIMEOUT_SECONDS,
    .tv_usec = 0
};

//functions
void init_session_thread_tracker(void);
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

        struct linger sl = {1, 2}; //wait max 2 seconds to send remaining data
        int setsockopt_result = setsockopt(*client_socket, SOL_SOCKET, SO_LINGER, (char*)&sl, sizeof(sl));
        if(setsockopt_result == SOCKET_ERROR) {
            fprintf(stderr, "setsockopt failed with %u\n", WSAGetLastError());
            send_error_pkt(*client_socket, MRMP_ERR_UNKNOWN);
            shutdown(*client_socket, SD_SEND);
            closesocket(*client_socket);
            free(client_socket);
        }

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

void init_session_thread_tracker(void) {
    for(size_t i = 0; i < MAX_SESSION_THREADS; ++i) {
        session_thread_tracker[i].thread_handle = NULL;
        session_thread_tracker[i].is_active = FALSE;   
    }
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
            printf(
                "Total connections since startup    : %d\n"
                "Active connections                 : %d\n\n"
                "Total sessions since startup       : %d\n"
                "Active sessions                    : %d\n",
            total_connections, active_connections, total_sessions, active_sessions);
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

    free(client_socket);
    _endthreadex(0);
    return 0;
}

//TODO: more error handling on send functions.
unsigned __stdcall do_session(void* session_state) {
    session_t* session = (session_t*) session_state;
    int stop_session = FALSE;

    //generate a maze for the session. 
    maze_size_t rows = 10;
    maze_size_t columns = 20; 
    maze_t* maze = generate_maze(rows, columns);
    maze_size_t winning_row = rows - 1;
    maze_size_t winning_column = columns - 1;

    //respond to both connected clients previously sent JOIN packets.
    send_join_resp_pkt(session->player_one, maze);
    send_join_resp_pkt(session->player_two, maze);

    //wait for ready packets.
    char* msg = NULL;
    int p1_receive_result = receive_mrmp_msg(session->player_one, &msg, &DEFAULT_TIMEOUT);
    if(p1_receive_result != SUCCESS) {
        if(p1_receive_result == TIMEDOUT) send_timeout_pkt(session->player_one);
        free(msg);
        cleanup_bad_session(session, maze, session->player_two, MRMP_ERR_UNKNOWN);
    }

    free(msg);

    int p2_receive_result = receive_mrmp_msg(session->player_two, &msg, &DEFAULT_TIMEOUT);
    if(p2_receive_result != SUCCESS) {
        if(p2_receive_result == TIMEDOUT) send_timeout_pkt(session->player_two);
        free(msg);
        cleanup_bad_session(session, maze, session->player_one, MRMP_ERR_UNKNOWN);
    }

    free(msg);

    //at this point, both clients have verified they are ready to start the race, so send a start packet to both.
    //I assume that there won't be too much delay between sequential sends.
    //TODO: would randomized send order make it slightly more fair?
    send_start_pkt(session->player_one);
    send_start_pkt(session->player_two);

    //start the session loop, wait for move or leave messages, any other message is considered illegal at this point in time.
    while(stop_session != TRUE) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(session->player_one, &read_fds);
        FD_SET(session->player_two, &read_fds);

        // Wait for socket to become readable.
        int select_result = select(0, &read_fds, NULL, NULL, &ACTIVITY_TIMEOUT);
        if(select_result == 0) {
            send_timeout_pkt(session->player_one);
            send_timeout_pkt(session->player_two);
            fprintf(stderr, "a session is timing out due to player inactivity\n.");
            stop_session = TRUE;
        } else if(select_result == SOCKET_ERROR) {
            fprintf(stderr, "a session is timing out due to a socket connection error.\n.");
            stop_session = TRUE;
        } else {
            for(u_int i = 0; i < read_fds.fd_count; ++i) {
                if(stop_session == TRUE) {
                    break;
                }

                SOCKET socket = read_fds.fd_array[i];
                maze_size_t* row_ptr = NULL;
                maze_size_t* column_ptr = NULL;

                if(socket == session->player_one) {
                    row_ptr = &session->player_one_row;
                    column_ptr = &session->player_one_column; 
                } else if(socket == session->player_two) {
                    row_ptr = &session->player_two_row;
                    column_ptr = &session->player_two_column; 
                }

                if(FD_ISSET(socket, &read_fds)) {
                    //read in message and interpret.
                    int receive_result = receive_mrmp_msg(socket, &msg, NULL);
                    if(receive_result != SUCCESS && receive_result != TIMEDOUT) {
                        stop_session = TRUE;
                    }
                    if(msg != NULL) {
                        switch(PHEADER(msg)->opcode) {
                            case MRMP_OPCODE_MOVE:
                                if(maze_is_move_valid(maze, *row_ptr, *column_ptr, PMOVE(msg)->row, PMOVE(msg)->column) == FALSE) {
                                    send_bad_move_pkt(socket, *row_ptr, *column_ptr);
                                    fprintf(stderr, "sent bad move packet.\n");
                                    free(msg);
                                    msg = NULL;
                                    continue;
                                }
                
                                //move was valid, update session state to reflect successful move, then notify other player socket to update
                                //their perspective of the current player socket's position in the maze.
                                *row_ptr = PMOVE(msg)->row;
                                *column_ptr = PMOVE(msg)->column;

                                send_opponent_move_pkt(socket_complement(socket, session), *row_ptr, *column_ptr);

                                if(*row_ptr == winning_row && *column_ptr == winning_column) {
                                    send_result_pkt(socket, 1);
                                    send_result_pkt(socket_complement(socket, session), 0);
                                    Sleep(200);
                                    stop_session = TRUE;
                                    break;
                                }
                                break;
                            case MRMP_OPCODE_LEAVE:
                                //notify other socket of current sockets desire to leave, give an unknown error due to unknown leave reason.
                                send_error_pkt(socket_complement(socket, session), MRMP_ERR_UNKNOWN);
                                stop_session = TRUE;
                                break;
                            default:
                                //illegal opcode received.
                                send_error_pkt(socket, MRMP_ERR_ILLEGAL_OPCODE);
                                stop_session = TRUE;
                                break;
                        };

                        free(msg);
                        msg = NULL;
                    }
                }
            }
        }
    }

    //TODO: wait for another JOIN packet if the client wants to play again.
    shutdown(session->player_one, SD_SEND);
    closesocket(session->player_one);
    shutdown(session->player_two, SD_SEND);
    closesocket(session->player_two);

    free(msg);
    free(session);
    free_maze(maze);

    _endthreadex(0);
    return 0;
}

unsigned __stdcall create_sessions(void* data) {
    //TODO: look into windows condition variables to conditionally enter critical sections here.
    while(quit != TRUE) {
        EnterCriticalSection(&player_queue_critsec);
        //check if there is at least 2 players waiting in queue.
        //TODO: does reading active_sessions need to be locked?
        while(player_queue_size(player_queue) >= 2 && active_sessions <= MAX_SESSION_THREADS) {
            SOCKET player_one_sock = *player_queue_front(player_queue);
            player_queue_pop(player_queue);
            SOCKET player_two_sock = *player_queue_front(player_queue);
            player_queue_pop(player_queue);

            //initialize the session's state. ownership of this pointer is passed onto the session thread that will be made.
            session_t* session = malloc(sizeof(session_t));
            session->player_one = player_one_sock;
            session->player_two = player_two_sock;
            session->player_one_row = session->player_one_column = session->player_two_row = session->player_two_column = 0;

            //spin up a thread to host the session for the 2 sockets.
            HANDLE session_thread = (HANDLE)_beginthreadex(NULL, 0, do_session, (void*) session, 0, NULL);
            if(session_thread == NULL) {
                fprintf(stderr, "failed to startup a session thread\n");
                free(session_thread);
                
                //push sockets back into player queue.
                player_queue_push(player_queue, player_one_sock);
                player_queue_push(player_queue, player_two_sock);

                continue;
            }
        }

        LeaveCriticalSection(&player_queue_critsec);
    }

    _endthreadex(0);
    return 0;
}