#ifndef HTTP_SERVER_HEADER
#define HTTP_SERVER_HEADER

#define METHOD_SIZE 8
#define REQ_TARGET_SIZE 128
#define PROTOCOL_SIZE 64

#define STATUS_LINE_SIZE 32
#define RESP_HEAD_SIZE 512
#define REP_HEAD_SIZE 512
#define RESP_DATA_SIZE 1024
#define RESP_SIZE STATUS_LINE_SIZE + RESP_HEAD_SIZE + REP_HEAD_SIZE + RESP_DATA_SIZE

typedef struct {
   char method[METHOD_SIZE];
   char req_target[REQ_TARGET_SIZE];
   char protocol[PROTOCOL_SIZE];
} http_req_T;

#endif
