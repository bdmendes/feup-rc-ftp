
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_URL_LENGTH 2048
#define MAX_USER_LENGTH 256
#define MAX_PASS_LENGTH 256
#define ANONYMOUS_USER "anonymous"
#define ANONYMOUS_USER_PASS "1234"
#define DEFAULT_PORT 21
#define DEFAULT_RESOURCE "/"

#define URL_CHARS "([[:alnum:]\\$\\.\\+\\*\\(\\),!'_-]+)|(%[[:xdigit:]]{2})"

void print_usage(char *inv) {
    fprintf(stdout,
            "%s ftp://[[user][:password]@]<host>[:port]/<url-to-file>\n", inv);
}

struct con_info {
    char addr[MAX_URL_LENGTH];
    char user[MAX_USER_LENGTH];
    char pass[MAX_PASS_LENGTH];
    int port;
};

/**
 * @brief Verifies an URL is a valid FTP URL
 *
 * @param url URL to verify
 * @return int 0 in case of success, -1 otherwise
 */
int validate_ftp_url(char *url) {
    static const char url_regex[] = "^(ftp|FTP)://"
                                    "(((" URL_CHARS ")*(:(" URL_CHARS ")*)?@)?)"
                                    "((" URL_CHARS ")+(\\.(" URL_CHARS ")+)+)"
                                    "((:[[:digit:]]{1,4})?)"
                                    "(/(" URL_CHARS ")*)*$";

    regex_t regex;

    if (regcomp(&regex, url_regex, REG_EXTENDED | REG_NOSUB)) { return -1; }

    if (regexec(&regex, url, 0, NULL, 0)) { return -1; }

    regfree(&regex);

    return 0;
}

int parse_url_con_info(char *url, struct con_info *con_info) {
    regex_t regex;
    regmatch_t pmatch[1];
    regmatch_t temp_match;

    static const char port[] = ":[[:digit:]]{1,4}[/]*";
    if (regcomp(&regex, port, REG_EXTENDED)) { return -1; }
    if (regexec(&regex, url, 1, pmatch, 0) == 0) {
        temp_match = pmatch[0];
        char temp[5];
        snprintf(temp, sizeof temp, "%.*s",
                 pmatch[0].rm_eo - pmatch[0].rm_so - 2,
                 &url[pmatch[0].rm_so + 1]);
        con_info->port = strtol(temp, NULL, 10);
    } else {
        temp_match.rm_so = -1;
        con_info->port = DEFAULT_PORT;
    }

    static const char address[] =
        "[/@](" URL_CHARS ")+(\\.(" URL_CHARS
        ")+)+(:[[:digit:]]{1,4})?(/(" URL_CHARS ")+)*[/]?";
    if (regcomp(&regex, address, REG_EXTENDED)) { return -1; }
    if (regexec(&regex, url, 1, pmatch, 0) == 0) {
        if (temp_match.rm_so != -1) {
            snprintf(con_info->addr, sizeof con_info->addr, "%.*s%.*s",
                     temp_match.rm_so - pmatch[0].rm_so - 1,
                     &url[pmatch[0].rm_so + 1],
                     pmatch[0].rm_eo - temp_match.rm_eo + 1,
                     &url[temp_match.rm_eo - 1]);
        } else {
            snprintf(con_info->addr, sizeof con_info->addr, "%.*s",
                     pmatch[0].rm_eo - pmatch[0].rm_so - 1,
                     &url[pmatch[0].rm_so + 1]);
        }

        temp_match = pmatch[0];
    }

    static const char username[] = "/(" URL_CHARS ")*[:@]";
    if (regcomp(&regex, username, REG_EXTENDED)) { return -1; }
    if (regexec(&regex, url, 1, pmatch, 0) == 0 &&
        pmatch[0].rm_so != temp_match.rm_so) {
        snprintf(con_info->user, sizeof con_info->user, "%.*s",
                 pmatch[0].rm_eo - pmatch[0].rm_so - 2,
                 &url[pmatch[0].rm_so + 1]);
    } else {
        snprintf(con_info->user, sizeof con_info->user, ANONYMOUS_USER);
    }

    static const char password[] = ":(" URL_CHARS ")*@";
    if (regcomp(&regex, password, REG_EXTENDED)) { return -1; }
    if (regexec(&regex, url, 1, pmatch, 0) == 0) {
        snprintf(con_info->pass, sizeof con_info->pass, "%.*s",
                 pmatch[0].rm_eo - pmatch[0].rm_so - 2,
                 &url[pmatch[0].rm_so + 1]);
    } else {
        snprintf(con_info->pass, sizeof con_info->pass, ANONYMOUS_USER_PASS);
    }

    regfree(&regex);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 1) {
        print_usage(argv[0]);
        return -1;
    }

    if (validate_ftp_url(argv[argc - 1]) != 0) {
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
