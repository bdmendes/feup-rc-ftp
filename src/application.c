#include <curl/curl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_URL_LENGTH 2048
#define MAX_USER_LENGTH 256
#define MAX_PASS_LENGTH 256
#define ANONYMOUS_USER "anonymous"
#define ANONYMOUS_USER_PASS "1234"
#define DEFAULT_PORT 21

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
 * @brief Decodes possibly encoded URLs
 *
 * @param url URL to decode
 * @param decoded_url Pointer to store the decoded URL
 * @param max_url_length Max decoded URL length
 * @return int 0 in case of success, -1 otherwise
 */
int decode_url(char *url, char *decoded_url, int max_url_length) {
    CURL *curl = curl_easy_init();
    if (curl) {
        int decodelen;
        char *decoded = curl_easy_unescape(curl, url, 0, &decodelen);
        if (decoded) {
            strncpy(decoded_url, decoded, max_url_length);
            curl_free(decoded);
        }
        curl_easy_cleanup(curl);
        return 0;
    }
    return -1;
}

int parse_url_con_info(char *url, struct con_info *con_info) {
    char host[MAX_URL_LENGTH];
    int delim[] = {0, 0, 0, 0, 0};
    int delim_c = 0;

    for (size_t i = 0; i < strlen(url); i++) {
        if (i == 0) {
            if (strncmp(url, "ftp://", 6) != 0) {
                // Message about missing or unsupported protocol
                return -1;
            } else {
                delim[0] = 5;
                delim_c++;
                i = 5;
            }
            continue;
        }

        if (delim_c < 5) {
            if (url[i] == ':' || url[i] == '@' ||
                (url[i] == '/' &&
                 (url[delim[delim_c - 1]] != '/' || delim_c - 1 == 0))) {
                delim[delim_c] = i;
                delim_c++;
            }
        } else {
            break;
        }
    }

    for (size_t i = 0; i < 5; i++) {
        if (url[delim[i]] == '/' && i != 0) {
            char temp_url[MAX_URL_LENGTH];
            snprintf(temp_url, MAX_URL_LENGTH, "%s%s", host, &url[delim[i]]);
            if (decode_url(temp_url, temp_url, sizeof temp_url) != 0) {
                return -1;
            }
            snprintf(con_info->addr, sizeof con_info->addr, "%s", temp_url);
            return 0;
        }

        if (url[delim[i]] == ':' && url[delim[i + 1]] == '/') {
            char temp_port[20];
            snprintf(temp_port, delim[i + 1] - delim[i], "%s",
                     &url[delim[i] + 1]);

            int port = 0;
            if ((port = atoi(temp_port)) == 0) {
                return -1;
            } else {
                con_info->port = port;
            }
        }

        if ((url[delim[i]] == '@' || url[delim[i]] == '/') &&
            url[delim[i + 1]] == '/') {
            con_info->port = 21;
        }

        if (url[delim[i]] == '@' &&
            (url[delim[i + 1]] == '/' || url[delim[i + 1]] == ':')) {
            snprintf(host, delim[i + 1] - delim[i], "%s", &url[delim[i] + 1]);
            if (strlen(host) == 0) { return -1; }
        }

        if (url[delim[i]] == '/' &&
            (url[delim[i + 1]] == '/' || url[delim[i + 1]] == ':')) {
            snprintf(host, delim[i + 1] - delim[i], "%s", &url[delim[i] + 1]);
            if (strlen(host) == 0) { return -1; }
            snprintf(con_info->user, sizeof con_info->user, "%s",
                     ANONYMOUS_USER);
            snprintf(con_info->pass, sizeof con_info->pass, "%s",
                     ANONYMOUS_USER_PASS);
        }

        if (url[delim[i]] == ':' && url[delim[i + 1]] == '@') {
            snprintf(con_info->user, delim[i] - delim[i - 1], "%s",
                     &url[delim[i - 1] + 1]);
            snprintf(con_info->pass, delim[i + 1] - delim[i], "%s",
                     &url[delim[i] + 1]);
        }

        if (url[delim[i]] == '/' && url[delim[i + 1]] == '@') {
            snprintf(con_info->user, delim[i + 1] - delim[i], "%s",
                     &url[delim[i] + 1]);
            if (strlen(con_info->user) == 0) { return -1; }
            snprintf(con_info->pass, sizeof con_info->pass, "%s",
                     ANONYMOUS_USER_PASS);
        }
    }
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc < 1) {
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
