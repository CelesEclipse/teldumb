#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "gateway/network/gwsocket.h"

#define LOOPBACK    "127.0.0.1"

int gw_socket_create(uint16_t port)
{
    int fd;

    if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0) {
        perror("Failed to initialize a TCP socket");
        return -1;
    }

    struct sockaddr_in addr;
    int reuse = 1;
    int err = 0;
    int qlen = 3;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
        goto errout;
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Failed to bind addr to fd");
        goto errout;
    }

    // listen
    if (listen(fd, qlen) < 0) {
        perror("Failed to listen to a connection");
        goto errout;
    }
    
    return fd;
    errout:
        err = errno;
        gw_socket_close(fd);
        errno = err;
        return -1;
}

int gw_socket_close(int fd)
{
    if (close(fd) < 0) return -1;
    return 0;
}

int gw_socket_accept(int fd)
{
    int err;
    int clfd = accept(fd, NULL, NULL);
    if (clfd < 0) {
        if (errno == EINTR) return -2;
        perror("Failed to accept a client connection");
        goto errout;
    }
    
    return clfd;
    errout:
        err = errno;
        errno = err;
        return -1;
}

ssize_t gw_socket_recv(int fd, void *buf, size_t len)
{
    int err;
    ssize_t bytes_recv;
    if ((bytes_recv = recv(fd, buf, len, 0)) < 0) {
        perror("recv");
        goto errout;
    } else if (bytes_recv == 0) {

    }

    return bytes_recv;
    errout:
        err = errno;
        errno = err;
        return -1;
}

ssize_t gw_socket_send(int fd, const void *buf, size_t len)
{
    int err;
    ssize_t bytes_sent;
    if ((bytes_sent = send(fd, buf, len, 0)) < 0) {
        perror("send");
        goto errout;
    }

    return bytes_sent;
    errout:
        err = errno;
        errno = err;
        return -1;
}
