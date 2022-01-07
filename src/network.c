#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "network.h"

int open_connect_socket(const char *address, const int port) {
    struct addrinfo hints;
    struct addrinfo *addrinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int status = getaddrinfo(address, "ftp", &hints, &addrinfo);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return -1;
    }

    int fd = socket(addrinfo->ai_family, addrinfo->ai_socktype,
                    addrinfo->ai_protocol);
    if (fd == -1) {
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
    int r = shutdown(fd, SHUT_RDWR);
    if (r == -1) {
        perror("shutdown");
    }
    return r;
}

int send_message(int socket_fd, char *message, int length) {
    int no_bytes = 0;
    while (no_bytes != length) {
        int s = send(socket_fd, message + no_bytes, length - no_bytes, 0);
        if (s == -1) {
            perror("send");
            return -1;
        }
        no_bytes += s;
    }
    return no_bytes;
}

int receive_message(int socket_fd, char *buf, int max_length) {
    int no_bytes = recv(socket_fd, buf, max_length - 1, 0);
    if (no_bytes == -1) {
        perror("recv");
    }
    buf[no_bytes] = '\0';
    return no_bytes;
}
