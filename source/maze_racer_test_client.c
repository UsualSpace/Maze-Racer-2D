// Filename: maze_racer_test_client.c
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/27/2025
// Purpose: To confirm messages are being properly sent back and forth.

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <conio.h>

#include "networking_utils.h"
#include "maze.h"

#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS 0
#endif //EXIT_SUCCESS
#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif //EXIT_FAILURE

#define PLAYER_CHAR 'o'

static int p1_row = 0;
static int p1_column = 0; 
static int last_p1_row = 0;
static int last_p1_column = 0;

static int p2_row = 0;
static int p2_column = 0;
static int last_p2_row = 0;
static int last_p2_column = 0;

static struct timeval DONT_BLOCK = {
    .tv_sec = 0,
    .tv_usec = 0
};

void process_input(void);
int changed_position(int is_p1);
void draw_player(int old_row, int old_column, int row, int column, int maze_start_row, int maze_start_column);

//assist in console rendering.
COORD get_cursor_position();
void move_cursor(int x, int y);


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

    //say hello to the server.
    int hello_result = send_hello_pkt(connect_socket, 0);
    printf("sent hello packet.\n");

    //wait for a hello acknowledgement from the server.
    char* msg = NULL;
    receive_mrmp_msg(connect_socket, &msg, NULL);
    if(PHEADER(msg)->opcode == MRMP_OPCODE_HELLO_ACK) {
        printf("Received hello acknowledgement packet!\n");
    }
    free(msg);

    //tell server you want to join the player queue to be put into a session.
    send_join_pkt(connect_socket);
    printf("sent join packet.\n");

    //wait for receival of the maze structure for rendering purposes.
    receive_mrmp_msg(connect_socket, &msg, NULL);
    if(PHEADER(msg)->opcode == MRMP_OPCODE_JOIN_RESP) {
        printf("Received join response + maze packet!\n");
    }

    //convert the flattened maze array into a 2D array.
    maze_t* maze = maze_network_to_host(PJOINRE(msg));
    free(msg);

    //draw the maze and save the position of its top left corner on screen.
    putchar('\n');
    COORD maze_origin = get_cursor_position();
    print_maze(maze);

    //send ready packet.
    send_ready_pkt(connect_socket);
    printf("sent ready packet.\n");

    //wait for start packet.
    int receive_start_result = receive_mrmp_msg(connect_socket, &msg, NULL);
    // if(receive_start_result == ) {

    // }

    if(PHEADER(msg)->opcode == MRMP_OPCODE_START) {
        printf("Received start notification! Beginning game loop.\n");
    }

    free(msg);

    msg = NULL;
    
    int stop_game = FALSE;

    //render players.
    draw_player(last_p1_row, last_p1_column, p1_row, p1_column, maze_origin.Y, maze_origin.X);
    draw_player(last_p2_row, last_p2_column, p2_row, p2_column, maze_origin.Y, maze_origin.X);

    while(stop_game != TRUE) {
        //read incoming messages first and foremost.
        int game_msg_result = receive_mrmp_msg(connect_socket, &msg, &DONT_BLOCK);
        if(game_msg_result != SUCCESS && game_msg_result != TIMEDOUT) {
            //TODO: better cleanup logic?
            stop_game = TRUE; //redunant but consistent.
            send_leave_pkt(connect_socket);
            break;
        }

        if(msg != NULL) {
            switch(PHEADER(msg)->opcode) {
                case MRMP_OPCODE_BAD_MOVE:
                    p1_row = PMOVE(msg)->row;
                    p1_column = PMOVE(msg)->column;
                    break;
                case MRMP_OPCODE_OPPONENT_MOVE:
                    p2_row = PMOVE(msg)->row;
                    p2_column = PMOVE(msg)->column;
                    if(changed_position(0) == TRUE) {
                        last_p2_row = p2_row;
                        last_p2_column = p2_column;
                    }
                    break;
                case MRMP_OPCODE_RESULT:
                    if(PRESULT(msg)->winner == 0) {
                        printf("You lost.\n");
                    } else {
                        printf("You won!\n");
                    }
                    stop_game = TRUE;
                    break;
                case MRMP_OPCODE_TIMEOUT:
                    printf("Timeout message received due to inactivity, aborting game session.\n");
                    stop_game = TRUE;
                    break;
                case MRMP_OPCODE_ERROR:
                    printf("Error message received, aborting game session.\n");
                    send_leave_pkt(connect_socket);
                    stop_game = TRUE;
                    break;
                default:
                    printf("Unknown message received, aborting game session.\n");
                    send_leave_pkt(connect_socket);
                    stop_game = TRUE;
                    break;
            };
        }

        free(msg);
        msg = NULL;

        if(stop_game == TRUE) break;
        
        //render players.
        if(changed_position(1) == TRUE)
            draw_player(last_p1_row, last_p1_column, p1_row, p1_column, maze_origin.Y, maze_origin.X);
        
        if(changed_position(0) == TRUE)
            draw_player(last_p2_row, last_p2_column, p2_row, p2_column, maze_origin.Y, maze_origin.X);
        
        process_input();

        if(changed_position(1) == TRUE) {
            send_move_pkt(connect_socket, p1_row, p1_column);
            last_p1_row = p1_row;
            last_p1_column = p1_column;
        }   
        //printf("\e[1;1H\e[2J");
        //printf("\e[H");
        
        Sleep(50);
    }
    
    shutdown(connect_socket, SD_SEND);
    closesocket(connect_socket);

    free_maze(maze);
    printf("exiting test client\n");
    return EXIT_SUCCESS;
}

void process_input(void) {
    int d;
    if(kbhit()) d = _getche();
    // int c;
    // while(c = getchar() != '\n' && c != EOF); //clear stdin.

    switch(d) {
        case 'w':
            p1_row -= 1;
            break;
        case 'a':
            p1_column -= 1;
            break;
        case 's':
            p1_row += 1;
            break;
        case 'd': 
            p1_column += 1;
            break;
        default:
            break;
    }
}

int changed_position(int is_p1) {
    if(is_p1 == TRUE)
        return (p1_row != last_p1_row || p1_column != last_p1_column);
    else {
        return (p2_row != last_p2_row || p2_column != last_p2_column);
    }
}

void draw_player(int old_row, int old_column, int row, int column, int maze_start_row, int maze_start_column) {
    COORD current_pos = get_cursor_position();
    
    //remove old position
    move_cursor((maze_start_column + old_column) * 3 + 1, (maze_start_row + old_row) * 3 + 1);
    putchar(' ');
    
    //draw in new position
    move_cursor((maze_start_column + column) * 3 + 1, (maze_start_row + row) * 3 + 1);
    putchar(PLAYER_CHAR);

    //move back to original position for further message printing.
    move_cursor(current_pos.X, current_pos.Y);
}

COORD get_cursor_position() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD pos = {0, 0};

    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        pos = csbi.dwCursorPosition;
    }

    return pos;
}

void move_cursor(int x, int y) {
    COORD pos = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}