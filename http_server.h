#ifndef HTTP_SERVER_HEADER
#define HTTP_SERVER_HEADER

#define METHOD_SIZE 8
#define REQ_TARGET_SIZE 128
#define HOST_SIZE 64

typedef struct {
   char method[METHOD_SIZE];
   char req_target[REQ_TARGET_SIZE];
   char host[HOST_SIZE];
} http_req_T;

#endif
