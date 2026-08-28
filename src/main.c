#include <stdio.h>
#include <stdlib.h>
#include "gateway/core/gateway.h"
#include "gateway/core/config.h"

#define PATH  "configs/gateway.conf"

int main(int argc, char ** argv)
{
    printf("config phase\n");
    GWConfig_t * cfg = config_alloc();
    
    if (cfg == NULL) {
        perror("Failed to alloc config");
        return 1;
    }
    if (config_load(cfg, PATH) != 0) return EXIT_FAILURE;
    if (config_validate(cfg) != 0) return EXIT_FAILURE; 

    config_destroy(cfg);
    return EXIT_SUCCESS;
}
