#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "network.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

int open_connect_socket(char *addr, int port) {
    struct addrinfo hints;
    struct addrinfo *addrinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int status;
    char port_[PATH_MAX];
    snprintf(port_, PATH_MAX, "%d", port);
    if ((status = getaddrinfo(addr, port_, &hints, &addrinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }

    int fd;
    if ((fd = socket(addrinfo->ai_family, addrinfo->ai_socktype,
                     addrinfo->ai_protocol)) == -1) {
        perror("socket");
        return -1;
    }

    if (connect(fd, addrinfo->ai_addr, addrinfo->ai_addrlen) == -1) {
        perror("connect");
        return -1;
    }

    return fd;
}

int close_socket(int fd) {
    if (shutdown(fd, SHUT_RDWR) == -1) {
        perror("shutdown");
        return -1;
    }
    return 0;
}

int send_msg(int socket_fd, char *msg) {
    int no_bytes = 0;
    int size = strnlen(msg, MAX_MSG_SIZE);
    if (msg[size - 1] != '\n') {
        msg[size++] = '\n';
    }
    while (no_bytes != size) {
        int s;
        if ((s = send(socket_fd, msg + no_bytes, size - no_bytes, 0)) == -1) {
            perror("send");
            return -1;
        }
        no_bytes += s;
    }
    return no_bytes;
}

int receive_msg(int socket_fd, char *buf) {
    int no_bytes;
    if ((no_bytes = recv(socket_fd, buf, MAX_MSG_SIZE - 1, 0)) == -1) {
        perror("recv");
    }
    buf[no_bytes] = '\0';
    return no_bytes;
}
