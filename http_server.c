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
#include "http_server.h"

#define LISTENING_ADDR "127.0.0.1"

//Global vars
char* err_msg;
char* err_loc;

//Kept global to allow signal handler to close fds properly
int server = -1; 
int client = -1;

void sigint_handler(int sig) {
    int curr_pid = getpid();
    printf("[PID %d] SIGINT received. Exiting.\n", curr_pid, sig);

    if (server >= 0) {
        close(server);
        printf("[PID %d] Server socket has been closed.\n", curr_pid);
    }
    if (client >= 0) {
        close(client);
        printf("[PID %d] Client socket has been closed.\n", curr_pid);
    }
    exit(0);
}

//Returns server fd on success, -1 on failure
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

//Return client fd on success, -1 on failure
int connect_to_client(int s) {
   //Client struct
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

//Returns pointer to struct for parsed HTTP request on success, 0 on failure
http_req_T* parse_request(char* req) {

    //Initializing req struct
    http_req_T* parsed_req;
    parsed_req = malloc(sizeof(http_req_T));    //Allocate memory to return struct
    memset(parsed_req, 0, sizeof(http_req_T)); //Set memory to null bytes

    //Extracting method from req
    char* p;
    for (p = req; *p != 0 && *p != ' '; p++);
    
    //Did not find whitespace
    if (*p != ' ') {
        err_msg = "Invalid request format";
        err_loc = "parse_request(): Extracting method";
        free(parsed_req);
        return 0;
    }

    *p = 0;
    snprintf(parsed_req -> method, METHOD_SIZE, req);

    //Extracting req target
    for (req = ++p; *p && *p != ' '; p++);

    if (*p != ' ') {
        err_msg = "Invalid request format";
        err_loc = "parse_request(): Extracting request target";
        free(parsed_req);
        return 0;
    }

    *p = 0;
    snprintf(parsed_req -> req_target, REQ_TARGET_SIZE, req);

    for (req = ++p; *p && *p != '\n'; p++) {
        if (*p == '\r') *p = 0;
    }

    
    if (*p != '\n') {
        err_msg = "Invalid request format";
        err_loc = "parse_request(): Extracting protocol";
        free(parsed_req);
        return 0;
    }  

    *p = 0;
    snprintf(parsed_req -> protocol, PROTOCOL_SIZE, req);
   
    return parsed_req;
}

//Returns 0 on success, -1 on failure 
int read_req(int c, char* buf, int bufsize) {
    int bytes_read = read(c, buf, bufsize-1);
    if (bytes_read < 0) {
        return -1;
    }
    buf[bytes_read] = 0; //Ensure buffer ends with null byte
    return 0;
}

void set_headers(int c, char* buf, const char* protocol, int status_code, const char* content_type, const char* data) {
   snprintf(buf, RESP_SIZE, 
           "%s %d OK\n"
           "Server: bl0nderServer\n"
           "Cache-Control: public; max-age=3600\n"
           "Content-Type: %s\n"
           "Content-Length: %d\n"
           "Content-Language: en\n"
           "\n%s\n",
           protocol, status_code, content_type, strlen(data), data);
}

//Returns 0 on success, -1 on failure
int send_response(int c, http_req_T* req) {
    
    //Initialize buffers
    char buf[RESP_SIZE];
    memset(buf, 0, RESP_SIZE);    
    
    //Set status line
    int status_code;

    //Success - 200 
    if (!strcmp(req -> method, "GET") && !strcmp(req -> req_target, "/test")) {
        set_headers(c, buf, req->protocol, 200, "text/html", 
                "<!DOCTYPE html>\n"
                "<html>\n"
                "<body><h1> HELLO WORLD! </h1></body>\n"
                "</html>\n");
    }
    //Page not found - 404
    else {
        set_headers(c, buf, req->protocol, 404, "text/plain", "Page not found :(");
    }

    //Write final response to client fd
    if (write(c, buf, strlen(buf)) < 0) {
        err_msg = "Error in writing response to client";
        err_loc = "send_response()";
        return -1;
    }

    return 0;
}


//Returns 0 on success, -1 on failure
int handle_connection(int s, int c) {
    
    //Initialize buffer for HTTP request
    int bufsize = 1024;
    char buf[bufsize];
    memset(buf, 0, bufsize);

    //Get client request
    if(read_req(c, buf, bufsize) < 0) {
        err_msg = "Empty HTTP request";
        err_loc = "handle_connection(): Reading client request";
        return -1;
    }
    /** printf("--------------------HTTP Request--------------------\n"); */
    /** printf("'%s'\n", buf); */

    //Parse request
    char* req = malloc(bufsize);
    memset(req, 0, bufsize);
    snprintf(req, bufsize-1, buf);
    
    http_req_T* parsed_req = parse_request(req);
    if (!parsed_req) {
        return -1;
    }

    /** printf("--------------------Parsed HTTP Request--------------------\n"); */
    /** printf("Method: '%s'\n", parsed_req -> method); */
    /** printf("Target: '%s'\n", parsed_req -> req_target); */
    /** printf("Protocol: '%s'\n", parsed_req -> protocol); */
    
    //Send reponse to client
    if (send_response(c, parsed_req) < 0) {
        return -1;
    } 

    return 0;
}

int main(int argc, char *argv[]) {

    //Variables
    int backlog = 2;
    int port = 1234; //default port

    if (argc >= 2) {
       port = atoi(argv[1]);
    } 

    //Signal handler
    signal(SIGINT, sigint_handler);

    server = create_server(port, backlog);
    if (server < 0){
        fprintf(stderr, "[!] %s in %s\n", err_msg, err_loc); 
        close(server);
        return -1;
    }
    
    printf("[PID %d] Listening on %s:%d...\n", getpid(), LISTENING_ADDR, port);

    while(1) {
        client = connect_to_client(server);
        if (client < 0) {
            fprintf(stderr, "[!] %s in %s\n", err_msg, err_loc);
            continue;
        }
        printf("[PID %d] Successfully connected with client with file descriptor %d.\n", getpid(), client);
        
        //Create child process to exchange data; parent process (server) keeps looking for connections
        if (fork() == 0) {

            //Error in handling connection with client
            if (handle_connection(server, client) < 0) {
                fprintf(stderr, "[!] %s in %s\n", err_msg, err_loc); 
                printf("[PID %d] Closing connection with client with file descriptor %d.\n", getpid(), client);
                close(client);
                exit(EXIT_FAILURE);
            }
                
            //No error
            printf("[PID %d] Closing connection with client with file descriptor %d.\n", getpid(), client);
            close(client);
            exit(EXIT_SUCCESS);
        }
        close(client);
    }

    return -1;
}
