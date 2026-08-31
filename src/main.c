#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "gateway/core/config.h"
#include "gateway/log/logger.h"
#include "gateway/system/gwsignal.h"

int main(int argc, char ** argv)
{
    if (argc < 3) {
        fprintf(stderr, "Use: %s <file_config> <file_log> \n", argv[0]);
        return EXIT_FAILURE;
    }

    char * cfg_path = argv[1];
    char * log_path = argv[2];
    int ret = EXIT_FAILURE;
    GWConfig_t * cfg = config_alloc();
    
    if (cfg == NULL) {
        fprintf(stderr, "Failed to allocate configuration\n");
        goto cleanup;
    }
    if (config_load(cfg, cfg_path) != 0) {
        fprintf(stderr, "Failed to load configuration\n");
        goto cleanup;
    }
    if (config_validate(cfg) != 0) {
        fprintf(stderr, "Invalid configuration\n");
        goto cleanup;
    }
    if (logger_init(log_path, config_get_level(cfg)) != 0) {
        fprintf(stderr, "Failed to initialize logger\n");
        goto cleanup;
    }
    
    if (gw_signal_init() != 0) {
        fprintf(stderr, "Failed to initialize signals\n");
        goto cleanup;
    }
    while (!gw_signal_should_shutdown()) {
        // polling later
        gw_signal_wait();
    }
    
    GW_LOG_INF("Gracefully shutdown...");
    ret = EXIT_SUCCESS;

    cleanup:
        logger_shutdown();
        config_destroy(cfg);
        return ret;
}
