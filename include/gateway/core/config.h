#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdlib.h>

typedef struct GWConfig GWConfig_t;

GWConfig_t * config_alloc();
int config_load(GWConfig_t * config, const char * path);
void config_destroy(GWConfig_t * config);
int config_validate(GWConfig_t * config);

// getters
uint8_t config_get_level(const GWConfig_t * config);

#endif
