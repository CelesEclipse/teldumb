#include <stdio.h>
#include <stdlib.h>
#include "gateway/core/gateway.h"
#include "gateway/core/config.h"
#include "gateway/log/logger.h"

#define CFG_PATH    "configs/gateway.conf"
#define LOG_PATH    "logs/gateway.log"

int main(int argc, char ** argv)
{
    logger_init(LOG_PATH, GW_LOG_DEBUG);
    LOG_INF("Starting ...");

    GWConfig_t * cfg = config_alloc();
    
    if (cfg == NULL) {
        GW_LOG_ERR("Failed to dynamically allocate the config");
        return 1;
    }
    if (config_load(cfg, CFG_PATH) != 0) GW_LOG_ERR("cannot load config in main");
    if (config_validate(cfg) != 0) GW_LOG_ERR("fail elsewhere");

    config_destroy(cfg);
    logger_shutdown();
    return EXIT_SUCCESS;
}
