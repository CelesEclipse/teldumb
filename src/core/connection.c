#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gateway/core/command.h"
#include "gateway/core/connection.h"

struct GWCnt_State
{
    int clfd;
    int client_id;
    GWConnection_State_t state;
};

GWCnt_State_t * gw_cntstate_alloc(void)
{
    GWCnt_State_t * cnt_state = malloc(sizeof(GWCnt_State_t));

    if (!cnt_state) return NULL;

    memset(cnt_state, 0, sizeof(GWCnt_State_t));
    return cnt_state;
}

void gw_cntstate_destroy(GWCnt_State_t * cnt_state)
{
    if (cnt_state != NULL) {
        free(cnt_state);
    }
    cnt_state = NULL;
}

int gw_cntstate_process(GWCnt_State_t * cnt, GWCmd_t * cmd_ptr, unsigned char * out_resp, int max_len, int * out_status)
{
    // 1. Safety Checks
    if (cnt == NULL || cmd_ptr == NULL || out_resp == NULL || out_status == NULL) {
        return -1;
    }

    GWCommand_t cmd = gw_parse_getcmd(cmd_ptr);
    int result = 0;

    // 2. Global Override: Disconnect is always allowed from any state
    if (cmd == DISCONNECT) {
        result = gw_parse_dispatch_cmd(cmd_ptr, out_resp, max_len);
        *out_status = 1; // Success
        return result; 
    }

    // 3. State Machine Guard Logic
    switch (cnt->state) {
        
        case STATE_CONNECTED: {
            if (cmd == REGISTER) {
                // Command is valid for this state -> Execute it!
                result = gw_parse_dispatch_cmd(cmd_ptr, out_resp, max_len);
                if (result > 0) {
                    cnt->state = STATE_REGISTERED; // Transition state
                    *out_status = 1;               // Success status
                } else {
                    *out_status = -1;              // Dispatcher internal failure
                }
            } else {
                // Unauthorized command for this state
                *out_status = -2; // Protocol violation: Must register first
                return -1;        // Return failure; do NOT call dispatch
            }
            break;
        }

        case STATE_REGISTERED: {
            if (cmd == AUTH) {
                result = gw_parse_dispatch_cmd(cmd_ptr, out_resp, max_len);
                if (result > 0) {
                    cnt->state = STATE_AUTHENTICATED;
                    *out_status = 1;
                } else {
                    *out_status = -1;
                }
            } else if (cmd == PING) {
                // If you want to allow PING at registered state
                result = gw_parse_dispatch_cmd(cmd_ptr, out_resp, max_len);
                *out_status = 1;
            } else {
                *out_status = -2; // Protocol violation: Must authenticate first
                return -1;
            }
            break;
        }

        case STATE_AUTHENTICATED: {
            switch (cmd) {
                case PING:
                case GET_STATUS:
                case SEND_DATA:
                    result = gw_parse_dispatch_cmd(cmd_ptr, out_resp, max_len);
                    if (result > 0) {
                        *out_status = 1;
                    } else {
                        *out_status = -1;
                    }
                    break;
                    
                case REGISTER:
                case AUTH:
                    *out_status = -3; // Redundant state action error
                    return -1;
                    
                default:
                    *out_status = -1; // Unknown command
                    return -1;
            }
            break; // Fixed missing break here!
        }

        default: {
            *out_status = -4; // Critical internal error state
            return -1;
        }
    }

    return result; // Returns total packet size on success
}


const char * gw_cntstate_string(GWCnt_State_t * cnt)
{
    if (!cnt) return "UNKNOWN";
    switch (cnt->state) {
        case STATE_CONNECTED: return "CONNECTED";
        case STATE_REGISTERED: return "REGISTERED";
        case STATE_AUTHENTICATED: return "AUTHENTICATED";
        default: return "INVALID";
    }
}
