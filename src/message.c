#include "message.h"

#include <ctype.h>
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

void parse_pasv_msg(char *msg, char *parsed) {
    char buf[MAX_MSG_SIZE];
    strncpy(buf, msg, sizeof buf);
    char ip[16];
    int a = -1;
    int b = -1;

    strtok(buf, "(,)");

    strncat(ip, strtok(NULL, "(,)"), 3);
    for (int i = 0; i < 3; i++) {
        strcat(ip, ".");
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

void parse_epsv_msg(char *msg, char *parsed) {
    char buf[MAX_MSG_SIZE];
    strncpy(buf, msg, sizeof buf);

    strtok(buf, "(|)");
    snprintf(parsed, MAX_ADDRESS_SIZE, "%s", strtok(NULL, "(|)"));
}
