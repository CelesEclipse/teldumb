#ifndef _GWSOCKET_H_
#define _GWSOCKET_H_

#include <stdint.h>

int gw_socket_create(uint16_t port);
int gw_socket_close(int fd);
int gw_socket_accept(int fd);

#endif
