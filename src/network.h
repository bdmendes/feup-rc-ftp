#pragma once

int open_connect_socket(const char *address, const int port);

int close_socket(int fd);

int send_message(int socket_fd, char *message, int length);

int receive_message(int socket_fd, char *buf, int max_length);
