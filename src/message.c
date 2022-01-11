#include "message.h"
#include "network.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ftp_code(char *msg) {
    char code[4] = "-1";
    int pos = 0;
    for (char *curr = msg; pos < 4 && *curr != '\0'; curr++) {
        if (!isdigit(*curr)) {
            break;
        }
        code[pos++] = *curr;
    }
    code[pos] = '\0';
    return atoi(code);
}

bool is_end_reply(char *msg) {
    bool new_line = true;
    bool found_space = false;
    for (char *curr = msg; *curr != '\0'; curr++) {
        if (*curr == '\n') {
            new_line = true;
            continue;
        }
        if (!isdigit(*curr) && new_line) {
            new_line = false;
            found_space = (*curr == ' ');
        }
    }
    return found_space;
}

bool parse_stat_reply_not_found(char *msg) {
    char msg_token_buf[MAX_BULK_DATA_MSG_SIZE];
    strncpy(msg_token_buf, msg, sizeof msg_token_buf);

    strtok(msg_token_buf, "\n");

    char *line = strtok(NULL, "\n");

    return (line[0] == '2');
}

bool parse_stat_reply_is_dir(char *msg) {
    char msg_token_buf[MAX_BULK_DATA_MSG_SIZE];
    strncpy(msg_token_buf, msg, sizeof msg_token_buf);

    strtok(msg_token_buf, "\n");

    char *line = strtok(NULL, "\n");

    return (line[0] == 'd');
}

size_t parse_stat_reply_size(char *msg) {
    char msg_token_buf[MAX_BULK_DATA_MSG_SIZE];
    strncpy(msg_token_buf, msg, sizeof msg_token_buf);

    strtok(msg_token_buf, "\n");

    char line_token_buf[MAX_BULK_DATA_MSG_SIZE];
    strncpy(line_token_buf, strtok(NULL, "\n"), sizeof line_token_buf);

    strtok(line_token_buf, " ");
    for (int i = 0; i < 3; i++) {
        strtok(NULL, " ");
    }

    char *size_token = strtok(NULL, " ");

    return strtoul(size_token, NULL, 10);
}

void parse_pasv_reply(char *msg, char *parsed) {
    char buf[MAX_CTRL_MSG_SIZE];
    strncpy(buf, msg, sizeof buf);
    char ip[16];
    int a = -1;
    int b = -1;

    strtok(buf, "(,)");

    strncat(ip, strtok(NULL, "(,)"), 3);
    for (int i = 0; i < 3; i++) {
        strncat(ip, ".", 2);
        strncat(ip, strtok(NULL, "(,)"), 3);
    }

    a = atoi(strtok(NULL, "(,)"));
    b = atoi(strtok(NULL, "(,)"));
    if (a < 0 || b < 0) {
        return;
    }
    a = 256 * a + b;

    snprintf(parsed, MAX_ADDRESS_SIZE, "%s:%d", ip, a);
}

void parse_epsv_reply(char *msg, char *parsed) {
    char buf[MAX_CTRL_MSG_SIZE];
    strncpy(buf, msg, sizeof buf);

    strtok(buf, "(|)");
    snprintf(parsed, MAX_ADDRESS_SIZE, "%s", strtok(NULL, "(|)"));
}
