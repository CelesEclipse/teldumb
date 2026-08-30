#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "gateway/core/config.h"
#include "gateway/log/logger.h"

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
        cfg->m_loglvl = 6;
    }
    return cfg;
}

int config_load(GWConfig_t * config, const char * path)
{
    int fd;

    if (config == NULL) {
        GW_LOG_ERR("Failed to load config file");
        return -1;
    }
    if ((fd = open(path, O_RDONLY)) < 0) {
        GW_LOG_ERR("Failed to open config file");
        return -1;
    }
    
    // read raw file and parse to struct
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read < 0) {
        GW_LOG_ERR("Failed to read config raw bytes");
        close(fd);
        return -1;
    }
    buffer[bytes_read] = '\0';
    close(fd);

    // parse
    char * line = strtok(buffer, "\n");
    while (line != NULL)  {
        if (strstr(line, "max_connections") != NULL) {
            sscanf(line, "max_connections = %d", &config->m_max_conn);
        } else if (strstr(line, "log_level") != NULL) {
            sscanf(line, "log = %hu", &config->m_loglvl);
        } else if (strstr(line, "server_port") != NULL) {
            sscanf(line, "port = %hu", &config->m_port);
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
    if (config == NULL) {
        GW_LOG_WRN("Null config");
        return -1;
    }
    if (config->m_port <= 0 || config->m_port > PORT_MAX) {
        GW_LOG_ERR("Invalid port %d", config->m_port);
        return -1;
    }
    if (config->m_max_conn <= 0) {
        GW_LOG_ERR("Max connections must be greater than 0");
        return -1;
    }
    return 0;
}
