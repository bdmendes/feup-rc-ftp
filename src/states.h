#pragma once

int login(int ctrl_socket_fd, char *user, char *pass);

int set_pasv_mode(int ctrl_socket_fd, char *pasv_addr);
