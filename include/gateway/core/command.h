#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <string.h>

#define HEADER_LEN      4
#define PAYLOAD_LEN     15
#define MAX_CMD_LEN     20
#define MAX_ARG_LEN     20
#define MAX_FRAME_LEN   256
#define MAX_BUF_LEN     1024

typedef enum
{
    REGISTER,
    AUTH,
    PING,
    GET_STATUS,
    SEND_DATA,
    DISCONNECT
} GWCommand_t;

typedef struct GWCmd GWCmd_t;

GWCmd_t *   gw_parse_alloc(void);
void        gw_parse_destroy(GWCmd_t * cmd);

int gw_parse_msglen_prefixing(const void * raw_stream,
                size_t available_bytes,
                size_t * out_consumed,
                char * out_payload,
                size_t max_payload_len);

int gw_parse_extract_cmd(const char * parsed_str, GWCmd_t * out_cmd);
int gw_parse_dispatch_cmd(const GWCmd_t *cmd, unsigned char * out_wire_buf, size_t max_buf_len);

#endif
