// Filename: maze_racer_client.c
// Programmer(s): Abdurrahman Alyajouri
// Date: 5/30/2025
// Purpose: The purpose of this file is create the main window and game logic of this application.

#include <glad/glad.h>
#include <GLFW/glfw3.h>
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

static int p1_row = 0;
static int p1_column = 0; 
static int p2_row = 0;
static int p2_column = 0;

static int WINDOW_WIDTH = 640;
static int WINDOW_HEIGHT = 480;
const char* WINDOW_TITLE = "INSERT TITLE HERE";

// prototypes.
void error_callback(int, const char*);
static void key_callback(GLFWwindow*, int, int, int, int);
static void framebuffer_callback(GLFWwindow*, int, int);

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
        fprintf(stderr, "unable to connect to MRMP server!\n");
        WSACleanup();
        return 1;
    }

    int send_hello_result = send_hello_pkt(connect_socket, 0);

    char* msg = NULL;
    int receive_hello_ack_result = receive_mrmp_msg(connect_socket, &msg, NULL);
    // if(receive_hello_ack_result == ) {

    // }
    
    if((PHEADER msg)->opcode == MRMP_OPCODE_HELLO_ACK) {
        printf("Received hello acknowledgement packet!\n");
        free(msg);
    } else {

    }

    send_join_pkt(connect_socket);

    receive_mrmp_msg(connect_socket, &msg, NULL);
    if(((mrmp_pkt_join_resp_t*) msg)->header.opcode == MRMP_OPCODE_JOIN_RESP) {
        printf("Received join response + maze packet!\n");
        free(msg);
    }
    
    
    //====================================================================================================
    //ALL GLFW AND OPENGL INITIALIZATION STUFF.

    glfwSetErrorCallback(error_callback);

    // initialize glfw, return on fail.
    if(!glfwInit()) return EXIT_FAILURE;

    // setup window and related hints/context.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if(!window) return EXIT_FAILURE;

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "failed to initialize GLAD\n");
        return EXIT_FAILURE;
    }   

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    //END OF GLFW AND OPENGL INITIALIZATION STUFF.
    //====================================================================================================

    // main loop
    while(!glfwWindowShouldClose(window)) {

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // dark gray background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        //TODO: game logic updates and rendering here.


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //cleanup.
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}

void error_callback(int error, const char* description) {
    fprintf("GLFW ERROR: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static void framebuffer_callback(GLFWwindow* window, int width, int height) {
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}