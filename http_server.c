#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>

#define LISTENING_ADDR "127.0.0.1"

char* err_msg;

int create_server(int port, int backlog) {
	
    //Create socket 
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		err_msg = "Error in creating socket.";
		return -1;
	}

	// Creating server struct
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(LISTENING_ADDR);
	
	// Bind socket to server 
	if (bind(s, (const struct sockaddr*) &server, sizeof(server)) < 0) {
		err_msg = "Error in binding socket.";
		return -1;	
	}
	
	//Listens for connections
	if (listen(s, backlog) < 0) {
		err_msg = "Error in listening for connections.";
		return -1;
	}

	return s;
}

int main() {
    //Variables
	int port = 1234;
    int backlog = 10;

	int server_socket_fd = create_server(port, backlog);
	if (server_socket_fd < 0){
		fprintf(stderr, err_msg);	
        close(server_socket_fd);
		return -1;
	}
	
	printf("Listening on port %d...\n", port);
	while(1) {
        //Put something here
	}

	return 0;
}
