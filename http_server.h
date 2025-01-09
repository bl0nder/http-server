#ifndef HTTP_SERVER_HEADER
#define HTTP_SERVER_HEADER

typedef struct {
   char method[8];
   char req_target[128];
   char host[64];
} http_req_T;

#endif
