#ifndef HTTP_SERVER_HEADER
#define HTTP_SERVER_HEADER

#define METHOD_SIZE 8
#define REQ_TARGET_SIZE 128
#define PROTOCOL_SIZE 64

#define RESPONSE_SIZE 1024
#define STATUS_LINE_SIZE 32
#define REP_HEADERS_SIZE 2048


typedef struct {
   char method[METHOD_SIZE];
   char req_target[REQ_TARGET_SIZE];
   char protocol[PROTOCOL_SIZE];
} http_req_T;

#endif
