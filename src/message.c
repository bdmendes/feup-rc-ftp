#include "message.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ftp_code(char *msg) {
    char code[20] = "-1";
    int pos = 0;
    for (char *curr = msg; pos < 20 && *curr != '\0'; curr++) {
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
