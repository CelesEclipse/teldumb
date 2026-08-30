#include <stdio.h>
#include <stdlib.h>
#include "gateway/core/config.h"
#include "gateway/log/logger.h"

#define CFG_PATH    "configs/gateway.conf"
#define LOG_PATH    "logs/gateway.log"

int main(int argc, char ** argv)
{
    GWConfig_t * cfg = config_alloc();
    
    if (cfg == NULL) return 1;
    config_load(cfg, CFG_PATH);
    config_validate(cfg);

    logger_init(LOG_PATH, config_get_level(cfg));
    GW_LOG_INF("Starting ...");

    logger_shutdown();
    config_destroy(cfg);
    return EXIT_SUCCESS;
}
