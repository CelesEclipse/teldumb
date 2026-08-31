#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "gateway/core/config.h"

#define PORT_MAX        65535
#define BUFFER_SIZE     1024
#define LOGLVL_SIZE     20

struct GWConfig
{
    uint16_t    m_port;
    uint32_t    m_max_conn;
    uint8_t     m_loglvl;
};

GWConfig_t * config_alloc()
{
    GWConfig_t * cfg = malloc(sizeof(GWConfig_t));
    if (cfg != NULL) {
        memset(cfg, 0, sizeof(GWConfig_t));
        cfg->m_loglvl = 0;
    }
    return cfg;
}

int config_load(GWConfig_t * config, const char * path)
{
    int fd;

    if (config == NULL) return -1;
    if ((fd = open(path, O_RDONLY)) < 0) return -1;
    
    // read raw file and parse to struct
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read < 0) {
        close(fd);
        return -1;
    }
    buffer[bytes_read] = '\0';
    close(fd);

    // parse
    char * line = strtok(buffer, "\n");
    while (line != NULL)  {
        if (strstr(line, "max_connections") != NULL) {
            if(sscanf(line, "max_connections = %u", &config->m_max_conn) == -1) return -1;
        } else if (strstr(line, "log_level") != NULL) {
            if(sscanf(line, "log_level = %hhu", &config->m_loglvl) == -1) return -1;
        } else if (strstr(line, "server_port") != NULL) {
            if(sscanf(line, "server_port = %hu", &config->m_port) == -1) return -1;
        }

        line = strtok(NULL, "\n");
    }

    return 0;
}

void config_destroy(GWConfig_t * config)
{
    if (config != NULL) {
        free(config);
    }
    config = NULL;
}

int config_validate(GWConfig_t * config)
{
    if (config == NULL) return -1;
    if (config->m_port <= 0 || config->m_port > PORT_MAX) return -1;
    if (config->m_max_conn <= 0) return -1;
    return 0;
}

uint8_t config_get_level(const GWConfig_t * config)
{
    return config->m_loglvl;
}

uint16_t config_get_port(const GWConfig_t * config)
{
    return config->m_port;
}
