#ifndef _GWSIGNAL_H_
#define _GWSIGNAL_H_

int gw_signal_init(void);
int gw_signal_should_shutdown(void);
void gw_signal_wait(void);

#endif
