#include <stdio.h>
#include <stdlib.h>
#include "gateway/core/gateway.h"
#include "gateway/core/config.h"

#define PATH  "configs/gateway.conf"

int main(int argc, char ** argv)
{
    printf("cc\n");
    GWConfig_t * cfg = config_alloc();
    
    if (cfg == NULL) {
        perror("Failed to alloc config");
        return 1;
    }
    if(config_load(cfg, PATH) == 0)
        printf("ok load\n");

    config_destroy(cfg);
    return 0;
}