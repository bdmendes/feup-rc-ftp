#pragma once

#include <stdbool.h>
#include <stdlib.h>

#define MAX_ADDRESS_SIZE 30

int ftp_code(char *msg);

bool is_end_reply(char *msg);

bool stat_reply_not_found(char *msg);

bool stat_reply_is_dir(char *msg);

size_t stat_reply_size(char *msg);

int parse_pasv_reply(char *msg, char *out_reply);

void parse_epsv_reply(char *msg, char *out_reply);
