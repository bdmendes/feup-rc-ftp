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
    if (!curl) { return -1; }
    char *decoded = curl_easy_unescape(curl, url, 0, NULL);
    if (!decoded) {
        curl_easy_cleanup(curl);
        return -1;
    }
    strncpy(decoded_url, decoded, max_url_length);
    curl_free(decoded);
    curl_easy_cleanup(curl);
    return 0;
}

int parse_url_con_info(char *url, struct con_info *con_info) {
    if (strncmp(url, "ftp://", 6) != 0) {
        fprintf(stderr, "Missing or unsuported protocol used.\n");
        return -1;
    }

    int url_start_idx = 0;
    for (size_t i = 6; i < strlen(url); i++) {
        if (url[i] == '/') {
            url_start_idx = i;
            break;
        }
    }

    if (url_start_idx == 0) { return -1; }

    char temp_url[MAX_URL_LENGTH];
    char temp_host[MAX_URL_LENGTH];
    snprintf(temp_url, url_start_idx - 5, "%s", &url[6]);

    char *tokens[] = {NULL, NULL, NULL, NULL};
    int token_c = 0;
    char *token = strtok(temp_url, "@:");

    while (token != NULL && token_c < 4) {
        tokens[token_c++] = token;
        token = strtok(NULL, "@:");
    }

    if (token_c < 1) { return -1; }

    snprintf(con_info->user, sizeof con_info->user, "%s", ANONYMOUS_USER);
    snprintf(con_info->pass, sizeof con_info->pass, "%s", ANONYMOUS_USER_PASS);
    con_info->port = DEFAULT_PORT;
    int port = 0;

    switch (token_c) {
        case 1: snprintf(temp_host, sizeof temp_host, "%s", tokens[0]); break;
        case 2:
            port = 0;
            if ((port = atoi(tokens[1])) != 0) {

                con_info->port = port;
                snprintf(temp_host, sizeof temp_host, "%s", tokens[0]);
            } else {

                snprintf(con_info->user, sizeof con_info->user, "%s",
                         tokens[0]);
                snprintf(temp_host, sizeof temp_host, "%s", tokens[1]);
            }
            break;
        case 3:
            port = 0;
            if ((port = atoi(tokens[2])) != 0) {
                con_info->port = port;
                snprintf(temp_host, sizeof temp_host, "%s", tokens[1]);
            } else {
                snprintf(con_info->pass, sizeof con_info->pass, "%s",
                         tokens[1]);
                snprintf(temp_host, sizeof temp_host, "%s", tokens[2]);
            }
            snprintf(con_info->user, sizeof con_info->user, "%s", tokens[0]);
            break;
        case 4:
            port = 0;
            if ((port = atoi(tokens[3])) == 0) { return -1; }
            con_info->port = port;
            snprintf(temp_host, sizeof temp_host, "%s", tokens[1]);
            snprintf(con_info->user, sizeof con_info->user, "%s", tokens[0]);
            snprintf(con_info->pass, sizeof con_info->pass, "%s", tokens[1]);
            snprintf(temp_host, sizeof temp_host, "%s", tokens[2]);
            break;

        default: break;
    }

    snprintf(temp_url, sizeof temp_url, "%s%s", temp_host, &url[url_start_idx]);
    if (decode_url(temp_url, con_info->addr, sizeof con_info->addr) != 0) {
        return -1;
    }
    return 0;
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
