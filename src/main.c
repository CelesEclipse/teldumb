#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gateway/core/config.h"
#include "gateway/log/logger.h"
#include "gateway/system/gwsignal.h"
#include "gateway/network/gwsocket.h"

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

    int listen_fd;
    if ((listen_fd = gw_socket_create(config_get_port(cfg))) < 0) {
        fprintf(stderr, "Failed to create gw socket in main\n");
        goto cleanup;
    }

    char buf[1024];
    
    while (!gw_signal_should_shutdown()) {
        // polling later
        int client_fd = gw_socket_accept(listen_fd);
        if (client_fd == -2) break;
        if (client_fd < 0) continue;

        GW_LOG_INF("Client connected . client_fd = %d", client_fd);
        while (1) {
            ssize_t bytes = gw_socket_recv(client_fd, buf, sizeof(buf) - 1);

            if (bytes < 0) {
                GW_LOG_ERR("Failed to receive from fd = %d", client_fd);
                break;
            }

            if (bytes == 0) {
                GW_LOG_INF("Client disconnected fd = %d", client_fd);
                break;
            }

            buf[bytes] = '\0';
            GW_LOG_INF("Received msg from client fd = %d, msg = %s", client_fd, buf);
            if (gw_socket_send(client_fd, buf, bytes) < 0) {
                GW_LOG_ERR("Failed to send to fd = %d", client_fd);
                break;
            }
            GW_LOG_INF("Sent msg to client fd = %d, msg = %s", client_fd, buf);
        }
        gw_socket_close(client_fd);
    }
    
    GW_LOG_INF("Gracefully shutdown...");
    ret = EXIT_SUCCESS;

    cleanup:
        logger_shutdown();
        config_destroy(cfg);
        return ret;
}
