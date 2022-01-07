#pragma once

#define MAX_URL_LENGTH 2048
#define MAX_USER_LENGTH 256
#define MAX_PASS_LENGTH 256

struct con_info {
    char addr[MAX_URL_LENGTH];
    char rsrc[MAX_URL_LENGTH];
    char user[MAX_USER_LENGTH];
    char pass[MAX_PASS_LENGTH];
    int port;
};

int validate_ftp_url(char *url);

int parse_url_con_info(char *url, struct con_info *con_info);
