#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "network.h"

int open_connect_socket(char *addr, char *port, int *ai_family) {
    struct addrinfo hints;
    struct addrinfo *addrinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int status;

    if ((status = getaddrinfo(addr, port, &hints, &addrinfo)) != 0) {
        freeaddrinfo(addrinfo);
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }

    int fd;
    if ((fd = socket(addrinfo->ai_family, addrinfo->ai_socktype,
                     addrinfo->ai_protocol)) == -1) {
        freeaddrinfo(addrinfo);
        perror("socket");
        return -1;
    }

    if (connect(fd, addrinfo->ai_addr, addrinfo->ai_addrlen) == -1) {
        freeaddrinfo(addrinfo);
        perror("connect");
        if (close(fd) == -1) {
            fprintf(stderr, "Error closing fd\n");
        }
        return -1;
    }

    if (ai_family != NULL) {
        *ai_family = addrinfo->ai_family;
    }

    freeaddrinfo(addrinfo);
    return fd;
}

int close_socket(int fd) {
    if (shutdown(fd, SHUT_RDWR) == -1) {
        perror("Shutdown");
        return -1;
    }
    if (close(fd) == -1) {
        perror("Close");
    }
    return 0;
}

int send_msg(int socket_fd, char *msg) {
    int no_bytes = 0;
    int size = strnlen(msg, MAX_CTRL_MSG_SIZE);
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

int receive_data(int socket_fd, int buf_size, char *buf, bool add_terminator) {
    int no_bytes;

    if (add_terminator) {
        if ((no_bytes = recv(socket_fd, buf, buf_size - 1, 0)) == -1) {
            perror("recv");
        }
        buf[no_bytes] = '\0';
    } else {
        if ((no_bytes = recv(socket_fd, buf, buf_size, 0)) == -1) {
            perror("recv");
        }
    }

    return no_bytes;
}
