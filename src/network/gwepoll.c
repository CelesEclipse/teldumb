#include "gateway/network/gwepoll.h"
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>

int gw_epoll_create(void)
{
    int ret = epoll_create1(0);
    if (ret < 0) {
        perror("Failed to create an epoll fd");
        return -1;
    }
    return ret;
}

int gw_epoll_add(int epfd, int fd, uint32_t events)
{
    struct epoll_event ev;

    ev.events = events;
    ev.data.fd = fd;

    return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}

int gw_epoll_remove(int epfd, int fd)
{
    return epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
}

int gw_epoll_wait(int epfd, struct epoll_event * events, int max_ev)
{
    if (!events) return -1;

    return epoll_wait(epfd, events, max_ev, -1);
}

int gw_epoll_close(int epfd)
{
    if (close(epfd) < 0) return -1;
    return 0;
}
