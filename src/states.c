#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "message.h"
#include "network.h"
#include "states.h"

int read_reply_code(int ctrl_socket_fd, char *msg) {
    int reply_code = -1;

    // Read the current message and get the reply code
    for (int tries = 0; tries < 3;) {
        if (receive_msg(ctrl_socket_fd, msg) != -1) {
            if (is_end_reply(msg)) {
                reply_code = ftp_code(msg);
                break;
            }
        } else {
            tries++;
        }
    }

    return reply_code;
}

int login(int ctrl_socket_fd, char *user, char *pass) {
    char msg[MAX_MSG_SIZE];
    int reply_code = -1;

    for (;;) {
        reply_code = -1;

        if ((reply_code = read_reply_code(ctrl_socket_fd, msg)) == -1) {
            return -1;
        }

        printf("code: %d\n", reply_code);

        switch (reply_code) {
            case 220: // Service ready for new user.
                snprintf(msg, sizeof msg, "user %s\n", user);
                break;

            case 331: // User name okay, need password.
                snprintf(msg, sizeof msg, "pass %s\n", pass);
                break;

            case 230: return 0; // Logged in
            case 500:           // Syntax error
            case 501:           // Syntax error in parameters or arguments
                return -2;
            case 332: // Need account to log in
                return -3;

            case 530: // Not logged in
            case 120: // Service ready in nnn minutes
            case 421: // Service not available, closing control connection.
            default: return -1;
        }

        if (send_msg(ctrl_socket_fd, msg) == -1) {
            return -1;
        }
    }
}

int set_pasv_mode(int ctrl_socket_fd, char *pasv_addr) {
    char msg[MAX_MSG_SIZE];
    int reply_code = -1;

    snprintf(msg, sizeof msg, "pasv\n");

    for (;;) {
        if (send_msg(ctrl_socket_fd, msg) == -1) {
            return -1;
        }

        if ((reply_code = read_reply_code(ctrl_socket_fd, msg)) == -1) {
            return -1;
        }

        printf("code: %d\n", reply_code);

        switch (reply_code) {
            case 227: // Entering Passive Mode (h1,h2,h3,h4,p1,p2).
                parse_pasv_msg(msg, pasv_addr);
                return 0;
            case 229: // Entering Extended Passive Mode (|||port|).
                parse_epsv_msg(msg, pasv_addr);
                return 1;
            case 500: // Syntax error
            case 501: // Syntax error in parameters or arguments
            case 502: // Command not implemented
                snprintf(msg, sizeof msg, "pasv\n");
                break;
            case 530: // Not logged in
            case 421: // Service not available, closing control connection.
            default: return -1;
        }
    }
}
