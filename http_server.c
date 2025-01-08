#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define LISTENING_ADDR "127.0.0.1"

char* err_msg;
char* err_loc;

int server_socket_fd = -1;

int create_server(int port, int backlog) {
	
    //Create socket 
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		err_msg = strerror(errno);
        err_loc = "Server socket:socket()";
		return -1;
	}

	// Creating server struct
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(LISTENING_ADDR);
	
	// Bind socket to server 
	if (bind(s, (const struct sockaddr*) &server, sizeof(server)) < 0) {
        err_msg = strerror(errno);
		err_loc = "Server socket:bind()";
		return -1;	
	}
	
	//Listens for connections
	if (listen(s, backlog) < 0) {
        err_msg = strerror(errno);
		err_loc = "Server socket:listen()";
		return -1;
	}

	return s;
}

void sigint_handler(int sig) {
    printf("SIGINT received. Exiting.\n", sig);

    if (server_socket_fd >= 0) {
        close(server_socket_fd);
        printf("Socket has been closed.\n");
    }
    exit(0);
}

int main() {
    
    //Variables
	int port = 1234;
    int backlog = 2;
    
    //Signal handler
    signal(SIGINT, sigint_handler);

	server_socket_fd = create_server(port, backlog);
	if (server_socket_fd < 0){
		fprintf(stderr, "[!]%s in %s\n", err_msg, err_loc);	
        close(server_socket_fd);
		return -1;
	}
	
	printf("Listening on port %d...\n", port);

    while(1) {
        //Put something here
    }

	return 0;
}
