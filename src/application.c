#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    return 0;
}
