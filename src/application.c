#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "network.h"
#include "url_parser.h"

void print_usage(char *app_name) {
    fprintf(stdout,
            "Usage: %s ftp://[[user][:password]@]<host>[:port]/<url-to-file>\n",
            app_name);
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

    printf("%s, %s, %s, %d\n", con_info.addr, con_info.user, con_info.pass,
           con_info.port);

    int socket_fd;
    if ((socket_fd = open_connect_socket(con_info.addr, con_info.port)) == -1) {
        return -1;
    }

    char buf[MAX_MSG_SIZE];
    for (int i = 0; i < 15; i++) {
        if (receive_msg(socket_fd, buf) != -1) {
            int c = ftp_code(buf);
            bool end = is_end_reply(buf);
            printf("\n###\n%s###code: %d, is_end: %d\n", buf, c, end);
            if (end) {
                printf("end! breaking\n");
                break;
            }
        } else {
            break;
        }
    }

    char msg[4096] = "user anonymous";
    int c = send_msg(socket_fd, msg);
    printf("\n\nsend_msg= %d\n", c);

    for (int i = 0; i < 15; i++) {
        if (receive_msg(socket_fd, buf) != -1) {
            int c = ftp_code(buf);
            bool end = is_end_reply(buf);
            printf("\n###\n%s###code: %d, is_end: %d\n", buf, c, end);
            if (end) {
                printf("end! breaking\n");
            }
        } else {
            break;
        }
    }

    if (close_socket(socket_fd) == 1) {
        return -1;
    }
    return 0;
}
