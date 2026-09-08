#ifndef _GWEPOLL_H_
#define _GWEPOLL_H_

#include <bits/stdint-uintn.h>
#include <sys/epoll.h>
#include <stdint.h>

int gw_epoll_create(void);
int gw_epoll_add(int epfd, int fd, uint32_t events);
int gw_epoll_remove(int epfd, int fd);
int gw_epoll_wait(int epfd, struct epoll_event * events, int max_ev);
int gw_epoll_close(int epfd);

#endif
