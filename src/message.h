#pragma once

#include <stdbool.h>

#ifndef MAX_MSG_SIZE
#define MAX_MSG_SIZE 4096
#endif

#define MAX_ADDRESS_SIZE 30

int ftp_code(char *msg);

bool is_end_reply(char *msg);

void parse_pasv_msg(char *msg, char *parsed);

void parse_epsv_msg(char *msg, char *parsed);
