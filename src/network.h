#pragma once

#include <stdbool.h>

#ifndef MAX_MSG_SIZE
#define MAX_MSG_SIZE 256000
#endif

int open_connect_socket(char *addr, char *port, int *ai_family);

int close_socket(int fd);

int send_msg(int socket_fd, char *msg);

int receive_msg(int socket_fd, char *buf, bool add_terminator);
