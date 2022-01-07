#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "url_parser.h"

#define ANONYMOUS_USER "anonymous"
#define ANONYMOUS_USER_PASS "1234"
#define DEFAULT_PORT 21
#define DEFAULT_RESOURCE "/"

#define URL_CHARS_REGEX                                                        \
    "([[:alnum:]\\$\\.\\+\\*\\(\\),!'_-]+)|(%[[:xdigit:]]{2})"

/**
 * @brief Verifies an URL is a valid FTP URL
 *
 * @param url URL to verify
 * @return int 0 in case of success, -1 otherwise
 */
int validate_ftp_url(char *url) {
    static const char url_regex[] =
        "^(ftp|FTP)://"
        "(((" URL_CHARS_REGEX ")*(:(" URL_CHARS_REGEX ")*)?@)?)"
        "((" URL_CHARS_REGEX ")+(\\.(" URL_CHARS_REGEX ")+)+)"
        "((:[[:digit:]]{1,4})?)"
        "(/(" URL_CHARS_REGEX ")+)*[/]?$";

    regex_t regex;
    if (regcomp(&regex, url_regex, REG_EXTENDED | REG_NOSUB) != 0) {
        regfree(&regex);
        return -1;
    }
    if (regexec(&regex, url, 0, NULL, 0) == REG_NOMATCH) {
        regfree(&regex);
        return -1;
    }
    regfree(&regex);
    return 0;
}

int parse_url_con_info(char *url, struct con_info *con_info) {
    regex_t regex;
    regmatch_t pmatch[1];
    regmatch_t temp_match;
    static const char port_regex[] = ":[[:digit:]]{1,4}[/]*";

    /* Compile regex */
    if (regcomp(&regex, port_regex, REG_EXTENDED) != 0) {
        regfree(&regex);
        return -1;
    }

    /* Parse port */
    if (regexec(&regex, url, 1, pmatch, 0) == 0) {
        temp_match = pmatch[0];
        char port_buf[5];
        snprintf(port_buf, sizeof port_buf, "%.*s",
                 pmatch[0].rm_eo - pmatch[0].rm_so - 2,
                 &url[pmatch[0].rm_so + 1]);
        con_info->port = strtol(port_buf, NULL, 10);
    } else {
        temp_match.rm_so = -1;
        con_info->port = DEFAULT_PORT;
    }

    /* Parse host address */
    static const char address[] =
        "[/@](" URL_CHARS_REGEX ")+(\\.(" URL_CHARS_REGEX
        ")+)+(:[[:digit:]]{1,4})?(/(" URL_CHARS_REGEX ")+)*[/]?";
    if (regcomp(&regex, address, REG_EXTENDED)) {
        regfree(&regex);
        return -1;
    }
    if (regexec(&regex, url, 1, pmatch, 0) == 0) {
        char tempAddr[MAX_URL_LENGTH];
        if (temp_match.rm_so != -1) {
            snprintf(tempAddr, sizeof tempAddr, "%.*s%.*s",
                     temp_match.rm_so - pmatch[0].rm_so - 1,
                     &url[pmatch[0].rm_so + 1],
                     pmatch[0].rm_eo - temp_match.rm_eo + 1,
                     &url[temp_match.rm_eo - 1]);
        } else {
            snprintf(tempAddr, sizeof tempAddr, "%.*s",
                     pmatch[0].rm_eo - pmatch[0].rm_so - 1,
                     &url[pmatch[0].rm_so + 1]);
        }

        char *resource_s = strchr(tempAddr, '/');
        if (resource_s != NULL) {
            snprintf(con_info->rsrc, sizeof con_info->rsrc, "%s", resource_s);
            snprintf(con_info->addr, sizeof con_info->addr, "%.*s",
                     (int)(resource_s - tempAddr), tempAddr);
        }
        temp_match = pmatch[0];
    }

    /* Parse username */
    static const char username[] = "/(" URL_CHARS_REGEX ")*[:@]";
    if (regcomp(&regex, username, REG_EXTENDED)) {
        return -1;
    }
    if (regexec(&regex, url, 1, pmatch, 0) == 0 &&
        pmatch[0].rm_so != temp_match.rm_so) {
        snprintf(con_info->user, sizeof con_info->user, "%.*s",
                 pmatch[0].rm_eo - pmatch[0].rm_so - 2,
                 &url[pmatch[0].rm_so + 1]);
    } else {
        snprintf(con_info->user, sizeof con_info->user, ANONYMOUS_USER);
    }

    /* Parse password */
    static const char password[] = ":(" URL_CHARS_REGEX ")*@";
    if (regcomp(&regex, password, REG_EXTENDED)) {
        regfree(&regex);
        return -1;
    }
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
