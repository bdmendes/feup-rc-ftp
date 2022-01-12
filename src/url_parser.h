#pragma once

#define MAX_URL_LENGTH 2048
#define MAX_USER_LENGTH 256
#define MAX_PASS_LENGTH 256
#define MAX_PORT_LENGTH 10

struct con_info {
    char addr[MAX_URL_LENGTH];
    char resource[MAX_URL_LENGTH];
    char user[MAX_USER_LENGTH];
    char pass[MAX_PASS_LENGTH];
    char port[MAX_PORT_LENGTH];
};

int validate_ftp_url(char *url);

int parse_url_con_info(char *url, struct con_info *con_info);
