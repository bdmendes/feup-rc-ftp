#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network.h"
#include "url_parser.h"

void print_usage(char *app_name) {
    fprintf(stdout,
            "Usage: %s ftp://[[user][:password]@]<host>[:port]/<url-to-file>\n",
            app_name);
}

int main(int argc, char *argv[]) {
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

    printf("%s, %s, %s, %d\n", con_info.addr, con_info.user, con_info.pass,
           con_info.port);

    int socket_fd = open_connect_socket(con_info.addr, con_info.port);
    if (socket_fd == -1) {
        return -1;
    }

    char buf[2048];
    buf[0] = 0;
    receive_message(socket_fd, buf, 2048);
    printf("buf: %s\n", buf);

    for (int i = 0; i < 7; i++) {
        receive_message(socket_fd, buf, 2048);
        printf("\nNEW MESSAGE------\n");
        printf("%s", buf);
        printf("\nEND MESSAGE----------\n");
        printf("i: %d", i);
        fflush(stdout);
    }

    char msg[2048] = "user anonymous";
    send_message(socket_fd, msg, 2048);
    printf("mandei: %s\n", msg);

    receive_message(socket_fd, buf, 2048);
    printf("buf: %s\n", buf);

    if (close_socket(socket_fd) == 1) {
        return -1;
    }
    return 0;
}
