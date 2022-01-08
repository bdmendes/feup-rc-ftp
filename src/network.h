#pragma once

#ifndef MAX_MSG_SIZE
#define MAX_MSG_SIZE 4096
#endif

int open_connect_socket(char *addr, char *port);

int close_socket(int fd);

int send_msg(int socket_fd, char *msg);

int receive_msg(int socket_fd, char *buf);
