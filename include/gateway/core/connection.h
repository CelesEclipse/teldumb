#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include "gateway/core/command.h"

typedef enum
{
    STATE_CONNECTED,
    STATE_REGISTERED,
    STATE_AUTHENTICATED
} GWConnection_State_t;

typedef struct GWCnt_State GWCnt_State_t;

GWCnt_State_t * gw_cntstate_alloc(void);
void gw_cntstate_destroy(GWCnt_State_t * cnt_state);

int gw_cntstate_process(GWCnt_State_t * cnt, GWCmd_t * cmd_ptr, unsigned char * out_resp, int max_len, int * out_status);
const char * gw_cntstate_string(GWCnt_State_t * cnt);

#endif
