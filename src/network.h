#pragma once

#include <stdbool.h>

#ifndef MAX_CTRL_MSG_SIZE
#define MAX_CTRL_MSG_SIZE 4096
#endif

#ifndef MAX_BULK_DATA_MSG_SIZE
#define MAX_BULK_DATA_MSG_SIZE 256000 // Around same as kernel
#endif
int open_connect_socket(char *addr, char *port, int *ai_family);

int close_socket(int fd);

int send_msg(int socket_fd, char *msg);

int receive_data(int socket_fd, int buf_size, char *buf, bool add_terminator);
