#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "message.h"
#include "network.h"
#include "states.h"
#include "url_parser.h"

int ctrl_socket_fd = -1;
int data_socket_fd = -1;

void print_usage(char *app_name) {
    fprintf(stdout,
            "Usage: %s ftp://[[user][:password]@]<host>[:port]/<url-to-file>\n",
            app_name);
}

static void close_ctrl_socket_fd() {
    if (close_socket(ctrl_socket_fd) == -1) {
        fprintf(stderr, "Failed to close control socket communications\n");
    }
    if (close(ctrl_socket_fd) == -1) {
        perror("Failed to close control socket fd");
    }
}

static void close_data_socket_fd() {
    if (close_socket(data_socket_fd) == -1) {
        fprintf(stderr, "Failed to close data socket communications\n");
    }
    if (close(data_socket_fd) == -1) {
        perror("Failed to close data socket fd");
    }
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    if (argc != 2) {
        print_usage(argv[0]);
        return -1;
    }

    if (validate_ftp_url(argv[1]) != 0) {
        print_usage(argv[0]);
        return -1;
    }

    struct con_info con_info;
    if (parse_url_con_info(argv[argc - 1], &con_info) != 0) {
        print_usage(argv[0]);
        return -1;
    }

    printf("%s, %s, %s, %s, %s\n", con_info.addr, con_info.rsrc, con_info.user,
           con_info.pass, con_info.port);

    /* Open data control socket */
    if ((ctrl_socket_fd = open_connect_socket(con_info.addr, con_info.port)) ==
        -1) {
        return -1;
    }

    /* Register atexit function related to control socket FD */
    if (atexit(close_ctrl_socket_fd) != 0) {
        fprintf(stderr, "Cannot register close_ctrl_socket_fd to run atexit\n");
        if (close(ctrl_socket_fd) != -1) {
            perror("Close control socket FD");
        }
    }

    /* Login in the server */
    int ret = 0;
    if ((ret = login(ctrl_socket_fd, con_info.user, con_info.pass)) != 0) {
        if (ret == -2) {
            fprintf(stdout, "Unrecognized user or password credentials\n");
        }
        if (ret == -3) {
            fprintf(stdout, "Unsupported login with this application\n");
        }
        return -1;
    }

    /* Set passive mode and open data socket */
    char pasv_addr[MAX_ADDRESS_SIZE];
    if ((ret = set_pasv_mode(ctrl_socket_fd, pasv_addr)) == -1) {
        return -1;
    }

    if (ret == 0) {
        char buf[MAX_ADDRESS_SIZE];
        char ip[MAX_URL_LENGTH];
        char port[MAX_PORT_LENGTH];
        strncpy(buf, pasv_addr, MAX_ADDRESS_SIZE);
        snprintf(ip, sizeof ip, "%s", strtok(buf, ":"));
        snprintf(port, sizeof port, "%s", strtok(NULL, ":"));

        if ((data_socket_fd = open_connect_socket(ip, port)) == -1) {
            return -1;
        }
    } else {
        if ((data_socket_fd = open_connect_socket(con_info.addr, pasv_addr)) ==
            -1) {
            return -1;
        }
    }

    /* Register atexit function related to data socket FD */
    if (atexit(close_data_socket_fd) != 0) {
        fprintf(stderr, "Cannot register close_data_socket_fd to run atexit\n");
        if (close(data_socket_fd) != -1) {
            perror("Close data socket FD");
        }
    }

    return 0;
}
