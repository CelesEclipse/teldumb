#include <bits/stdint-uintn.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gateway/core/config.h"
#include "gateway/core/command.h"
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
    
    while (!gw_signal_should_shutdown()) {
        // polling later
        int client_fd = gw_socket_accept(listen_fd);
        if (client_fd == -2) break;
        if (client_fd < 0) continue;

        GW_LOG_INF("Client connected . client_fd = %d", client_fd);

        // initialize stream buffer for client
        uint8_t stream_buffer[8 * MAX_BUF_LEN];
        size_t buffer_size = 0;

        while (1) {
            uint8_t temp_rx_buf[MAX_BUF_LEN];
            ssize_t bytes = gw_socket_recv(client_fd, temp_rx_buf, sizeof(temp_rx_buf));

            if (bytes < 0) {
                GW_LOG_ERR("Failed to receive from fd = %d", client_fd);
                break;
            }

            if (bytes == 0) {
                GW_LOG_INF("Client disconnected fd = %d", client_fd);
                break;
            }

            // Append received bytes to our urgh.. stream buf
            if (buffer_size + (size_t)bytes > sizeof(stream_buffer)) {
                GW_LOG_ERR("Buffre overflow occured on fd = %d! Dropping client ...", client_fd);
                break;
            }
            GW_LOG_INF("Received data from client, fd = %d, size = %zu", client_fd, (size_t)bytes);
            memcpy(stream_buffer + buffer_size, temp_rx_buf, (size_t)bytes);
            buffer_size += (size_t)bytes;

            // Loop through the memory, it seems ... a single recv() call might pulled multipackets
            while (1) {
                char clean_payload[MAX_FRAME_LEN];
                size_t total_bytes_consumed = 0;

                int frame_status = gw_parse_msglen_prefixing(stream_buffer, buffer_size,
                                                            &total_bytes_consumed, clean_payload, sizeof(clean_payload));
                
                if (frame_status == 0) break; // wait for more
                if (frame_status < 0) {
                    GW_LOG_ERR("Invalid or corrupt frame length for client fd = %d", client_fd);
                    break;
                }

                // Parse phase
                GWCmd_t * current_cmd = gw_parse_alloc();
                if (gw_parse_extract_cmd(clean_payload, current_cmd) == 1) {
                    uint8_t tx_buf[4 + MAX_BUF_LEN];
                    int out_pkt_size = gw_parse_dispatch_cmd(current_cmd, tx_buf, sizeof(tx_buf));

                    if (out_pkt_size > 0) {
                        gw_socket_send(client_fd, tx_buf, out_pkt_size);
                        GW_LOG_INF("Sent data to the client, fd = %d, size = %u", client_fd, out_pkt_size);
                    }
                }
                
                // buffer compaction
                size_t remaining_bytes = buffer_size - total_bytes_consumed;
                if (remaining_bytes > 0) {
                    memmove(stream_buffer, stream_buffer + total_bytes_consumed, remaining_bytes);
                }
                buffer_size = remaining_bytes;
                gw_parse_destroy(current_cmd);
            }
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
