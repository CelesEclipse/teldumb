#ifndef _CONFIG_H_
#define _CONFIG_H_

typedef struct GWConfig GWConfig_t;

GWConfig_t * config_alloc();
int config_load(GWConfig_t * config, const char * path);
void config_destroy(GWConfig_t * config);

#endif
