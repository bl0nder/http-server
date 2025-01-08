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

int server = -1;
int client = -1;

void sigint_handler(int sig) {
    int curr_pid = getpid();
    printf("[PID %d] SIGINT received. Exiting.\n", curr_pid, sig);

    if (server >= 0) {
        close(server);
        printf("[PID %d] Server socket has been closed.\n", curr_pid);
    }
    exit(0);
}

int create_server(int port, int backlog) {  
    //Create socket 
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        err_msg = strerror(errno);
        err_loc = "create_server():socket()";
        return -1;
    }

    // Creating server struct
    struct sockaddr_in s_struct;
    memset(&s_struct, 0, sizeof(s_struct)); //Ensure no random bytes are in struct after initialization
    
    s_struct.sin_family = AF_INET;
    s_struct.sin_port = htons(port);
    s_struct.sin_addr.s_addr = inet_addr(LISTENING_ADDR);
    
    // Bind socket to server
    if (bind(s, (const struct sockaddr*) &s_struct, sizeof(s_struct)) < 0) {
        err_msg = strerror(errno);
        err_loc = "create_server():bind()";
        return -1;  
    }
    
    //Listens for connections
    if (listen(s, backlog) < 0) {
        err_msg = strerror(errno);
        err_loc = "create_server():listen()";
        return -1;
    }

    return s;
}

int connect_to_client(int s) {
   struct sockaddr_in c_struct; 
   memset(&c_struct, 0, sizeof(c_struct));
   socklen_t addrlen = 0;
 
   int c = accept(s, (struct sockaddr*) &c_struct, &addrlen);
   if (c < 0) {
       err_msg = strerror(errno);
       err_loc = "connect_to_client():accept()";
       return -1;
   }
   
   return c;
}

void handle_connection(int s, int c) {
    sleep(3);    
    return;
}

int main() {
    
    //Variables
    int port = 1234;
    int backlog = 2;
    
    //Signal handler
    signal(SIGINT, sigint_handler);

    server = create_server(port, backlog);
    if (server < 0){
        fprintf(stderr, "[!] %s in %s\n", err_msg, err_loc); 
        close(server);
        return -1;
    }
    
    printf("Listening on %s:%d...\n", LISTENING_ADDR, port);

    while(1) {
        client = connect_to_client(server);
        if (client < 0) {
            fprintf(stderr, "[!] %s in %s\n", err_msg, err_loc);
            continue;
        }
        printf("Successfully connected with client %d.\n", client);
        
        //Create child process to exchange data; parent process (server) keeps looking for connections
        if (fork() == 0) {
            handle_connection(server, client);
            printf("[PID %d] Closing connection with client %d.\n", getpid(), client);
            close(client);
            exit(0);
        }
    }

    return -1;
}
