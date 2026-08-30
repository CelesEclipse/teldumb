#include <stdio.h>
#include <stdlib.h>
#include "gateway/core/config.h"
#include "gateway/log/logger.h"
#include "gateway/system/gwsignal.h"

#define CFG_PATH    "configs/gateway.conf"
#define LOG_PATH    "logs/gateway.log"

int main(int argc, char ** argv)
{
    int ret = EXIT_FAILURE;
    GWConfig_t * cfg = config_alloc();
    
    if (cfg == NULL) {
        fprintf(stderr, "Failed to allocate configuration\n");
        goto cleanup;
    }
    if (config_load(cfg, CFG_PATH) != 0) {
        fprintf(stderr, "Failed to load configuration\n");
        goto cleanup;
    }
    if (config_validate(cfg) != 0) {
        fprintf(stderr, "Invalid configuration\n");
        goto cleanup;
    }
    if (logger_init(LOG_PATH, config_get_level(cfg)) != 0) {
        fprintf(stderr, "Failed to initialize logger\n");
        goto cleanup;
    }
    
    if (gw_signal_init() != 0) {
        fprintf(stderr, "Failed to initialize signals\n");
        goto cleanup;
    }
    while (!gw_signal_should_shutdown()) {
        GW_LOG_INF("Starting ...");
    }
    
    ret = EXIT_SUCCESS;

    cleanup:
        logger_shutdown();
        config_destroy(cfg);
        return ret;
}
