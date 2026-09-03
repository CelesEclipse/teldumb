#include <bits/stdint-uintn.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include "gateway/core/command.h"

/* Leng-prefixing method 
    TCP is only a stream of bytes not packets,
    define a 4-byte integer as the prefix of the msg
    ┌───────────────────────────┬──────────────────────────────────┐
    │ Length header (4 bytes)   │ Payload      (Variable Length)   │
    │ uint32_t (Network Order)  │ "REGISTER user123\0"             │
    └───────────────────────────┴──────────────────────────────────┘
*/

struct GWCmd
{
    GWCommand_t command;
    uint32_t    arg_len;
    char        argument[MAX_ARG_LEN];    
};

GWCmd_t * gw_parse_alloc(void)
{
    GWCmd_t * cmd = malloc(sizeof(GWCmd_t));
    if (cmd != NULL) {
        memset(cmd, 0, sizeof(GWCmd_t));
    }
    return cmd;
}

void gw_parse_destroy(GWCmd_t *cmd)
{
    if (cmd != NULL) {
        free(cmd);
    }
    cmd = NULL;
}

int gw_parse_msglen_prefixing(const void * raw_stream,
                size_t available_bytes,
                size_t * out_consumed,
                char * out_payload,
                size_t max_payload_len)
{
    int err;

    // check 1: whether recv() enough 4 bytes
    if (available_bytes < HEADER_LEN)   goto errout;

    // check 2: cast 4-byte length header
    uint32_t network_len;
    memcpy(&network_len, raw_stream, HEADER_LEN);
    uint32_t payload_len = ntohl(network_len);

    // check 3: buffer overflows
    if (payload_len >= max_payload_len) return -1;

    // check 4: whether complete payload in stream yet
    size_t total_bytes = HEADER_LEN + payload_len;
    if (available_bytes < total_bytes) goto errout;

    const char * payload_start = (const char *)raw_stream + HEADER_LEN;
    memcpy(out_payload, payload_start, payload_len);
    out_payload[payload_len] = '\0';
    *out_consumed = total_bytes;
    
    return 1;

    errout:
        err = errno;
        errno = err;
        return 0;
}

int gw_parse_extract_cmd(const char * parsed_str, GWCmd_t * out_cmd)
{
    int err;
    int tokens;
    char cmd_token[MAX_CMD_LEN];
    char arg_token[MAX_ARG_LEN];

    if (out_cmd == NULL || parsed_str == NULL) goto errout;
    if ((tokens = sscanf(parsed_str, "%s %s", cmd_token, arg_token)) == -1) return -1;

    if (strcasecmp(cmd_token, "REGISTER") == 0) out_cmd->command = REGISTER;
    else if (strcasecmp(cmd_token, "AUTH") == 0) out_cmd->command = AUTH;
    else if (strcasecmp(cmd_token, "PING") == 0) out_cmd->command = PING;
    else if (strcasecmp(cmd_token, "GET_STATUS") == 0) out_cmd->command = GET_STATUS;
    else if (strcasecmp(cmd_token, "SEND_DATA") == 0) out_cmd->command = SEND_DATA;
    else if (strcasecmp(cmd_token, "DISCONNECT") == 0) out_cmd->command = DISCONNECT; 
    else goto errout;

    if (out_cmd->command == REGISTER || out_cmd->command == AUTH) {
        if (tokens != 2) goto errout;

        snprintf(out_cmd->argument, MAX_ARG_LEN, "%s", arg_token);
    }
    
    return 1;

    errout:
        err = errno;
        errno = err;
        return 0;
}

int gw_parse_dispatch_cmd(const GWCmd_t *cmd, unsigned char * out_wire_buf, size_t max_buf_len)
{
    if (cmd == NULL || out_wire_buf == NULL || max_buf_len < HEADER_LEN) return -1;

    char payload_buf[MAX_BUF_LEN];
    int dynamic_len = 0;

    switch ((int)cmd->command) {
        case REGISTER: {
            dynamic_len = snprintf(payload_buf, sizeof(payload_buf), "OK REGISTERED %s", cmd->argument);
            break;
        }
        case AUTH: {
            dynamic_len = snprintf(payload_buf, sizeof(payload_buf), "OK AUTH %s", cmd->argument);
            break;
        }
        case PING: {
            dynamic_len = snprintf(payload_buf, sizeof(payload_buf), "PONG");
            break;
        }
        case GET_STATUS: {
            dynamic_len = snprintf(payload_buf, sizeof(payload_buf), "STATUS 200");
            break;
        }
        case SEND_DATA: {
            dynamic_len = snprintf(payload_buf, sizeof(payload_buf), "SENT OK");
            break;
        }
        case DISCONNECT: {
            dynamic_len = snprintf(payload_buf, sizeof(payload_buf), "BYE BYE");
            break;
        }
        default:
            return -1;
    }

    // check1: snprintf truncate or fail
    if (dynamic_len <= 0 || (size_t)dynamic_len >= sizeof(payload_buf)) {
        return -1; 
    }

    // check2: whether output buffer have enough space for 4 + text
    size_t total_packet_size = HEADER_LEN + (size_t)dynamic_len;
    if (total_packet_size > max_buf_len) return -1;

    uint32_t network_len = htonl((uint32_t)dynamic_len);
    memcpy(out_wire_buf, &network_len, HEADER_LEN);
    memcpy(out_wire_buf + HEADER_LEN, payload_buf, (size_t)dynamic_len);

    return (int)total_packet_size;
}
