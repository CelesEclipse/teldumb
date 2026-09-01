#ifndef _GWSOCKET_H_
#define _GWSOCKET_H_

#include <stdint.h>
#include <unistd.h>

int gw_socket_create(uint16_t port);
int gw_socket_close(int fd);
int gw_socket_accept(int fd);
ssize_t gw_socket_recv(int fd, void *buf, size_t len);
ssize_t gw_socket_send(int fd, const void *buf, size_t len);

#endif
