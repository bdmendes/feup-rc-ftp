#pragma once

#include <aio.h>

int login(int ctrl_socket_fd, char *user, char *pass);

int get_file_size(int ctrl_socket_fd, char *path, size_t *size);

int set_pasv_mode(int ctrl_socket_fd, int ctr_socket_family, char *pasv_addr);

int init_retrieve(int ctr_socket_fd, char *path);

int end_retrieve(int ctrl_socket_fd);

int transfer_data(int data_socket_fd, int data_file_fd, size_t size);

int logout(int ctrl_socket_fd);
