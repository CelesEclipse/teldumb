#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "gateway/system/gwsignal.h"

volatile sig_atomic_t keep_running = 1;

static void shutdown_hdl(int signum)
{
    (void)signum;
    fprintf(stdout, "Gracefully shutdown ...\n");
    keep_running = 0;
}

int gw_signal_init(void)
{
    struct sigaction sa;
    sa.sa_handler = shutdown_hdl;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) < 0) return -1;
    if (sigaction(SIGTERM, &sa, NULL) < 0) return -1;

    return 0;
}

int gw_signal_should_shutdown(void)
{
    return keep_running == 0;
}
