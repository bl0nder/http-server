#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>

#define LISTENING_ADDR "127.0.0.1"
#define LISTENING_PORT 1234

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
	int b = bind(s, (const struct sockaddr*) &server, sizeof(server));
	if (b < 0) {
		err_msg = "Error in binding socket.";
		return -1;	
	}
	
	//Listens for connections
	int l = listen(s, backlog);
	if (l < 0) {
		err_msg = "Error in listening for connections.";
		return -1;
	}


	return s;
}

int main() {
	int port = 1234;
	int server_socket_fd = create_server(port, 10);
	if (server_socket_fd < 0){
		fprintf(stderr, err_msg);	
		return -1;
	}
	
	printf("Listening on port %d...\n", port);
	while(1) {

	}

	return 0;
}
