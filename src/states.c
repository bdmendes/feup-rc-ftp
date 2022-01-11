#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>

#include "message.h"
#include "network.h"
#include "states.h"
#include "utils.h"

int read_reply_code(int ctrl_socket_fd, char *msg) {
    // Read the current message and get the reply code
    for (int tries = 0; tries < 3;) {
        if (receive_data(ctrl_socket_fd, MAX_CTRL_MSG_SIZE, msg, true) != -1) {
            if (is_end_reply(msg)) {
                return ftp_code(msg);
            }
        } else {
            tries++;
        }
    }
    return -1;
}

int read_msg(int ctrl_socket_fd, int msg_size, char *msg) {
    int total_read_bytes = 0;
    int no_bytes = 0;
    char buf[MAX_CTRL_MSG_SIZE];

    for (int tries = 0; tries < 3;) {
        no_bytes = 0;
        if ((msg_size - total_read_bytes - 1) <= 0) {
            return 0;
        }

        int max_len = MIN(msg_size - total_read_bytes - 1, sizeof buf);

        if ((no_bytes = receive_data(ctrl_socket_fd, max_len, buf, true)) ==
            -1) {
            tries++;
            continue;
        }

        strncat(msg, buf, msg_size - total_read_bytes - 1);
        total_read_bytes += no_bytes;

        if (is_end_reply(msg)) {
            return ftp_code(msg);
        }
    }

    return -1;
}

int login(int ctrl_socket_fd, char *user, char *pass) {
    char msg[MAX_CTRL_MSG_SIZE];
    int reply_code = -1;

    for (;;) {
        reply_code = -1;

        if ((reply_code = read_reply_code(ctrl_socket_fd, msg)) == -1) {
            return -1;
        }

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

int get_file_size(int ctrl_socket_fd, char *path, size_t *size) {
    char msg[MAX_BULK_DATA_MSG_SIZE];
    int reply_code = -1;

    for (;;) {
        snprintf(msg, sizeof msg, "stat %s\n", path);

        if (send_msg(ctrl_socket_fd, msg) == -1) {
            return -1;
        }

        msg[0] = '\0';
        if ((reply_code = read_msg(ctrl_socket_fd, sizeof msg, msg)) == -1) {
            return -1;
        }

        switch (reply_code) {
            case 212: // Directory status.
            case 213: // File status.
                if (parse_stat_reply_not_found(msg)) {
                    return -2;
                }

                if (parse_stat_reply_is_dir(msg)) {
                    return -3;
                }
                *size = parse_stat_reply_size(msg);
                return 0;
            case 450: // Requested file action not taken.
                break;
            case 211: // System status, or system help reply.
            case 500: // Syntax error, command unrecognized. This may include
                      // errors such as command line too long.
            case 501: // Syntax error in parameters or arguments.
            case 502: // Command not implemented.
            case 421: // Service not available, closing control connection.
            case 530: // Not logged in.
            default: return -1;
        }
    }
}

int set_pasv_mode(int ctrl_socket_fd, int ai_family, char *pasv_addr) {
    char msg[MAX_CTRL_MSG_SIZE];
    int reply_code = -1;

    if (ai_family == AF_INET) {
        snprintf(msg, sizeof msg, "pasv\n");
    } else {
        snprintf(msg, sizeof msg, "epsv\n");
    }

    if (send_msg(ctrl_socket_fd, msg) == -1) {
        return -1;
    }

    if ((reply_code = read_reply_code(ctrl_socket_fd, msg)) == -1) {
        return -1;
    }

    switch (reply_code) {
        case 227: // Entering Passive Mode (h1,h2,h3,h4,p1,p2).
            parse_pasv_reply(msg, pasv_addr);
            return 0;
        case 229: // Entering Extended Passive Mode (|||port|).
            parse_epsv_reply(msg, pasv_addr);
            return 1;
        case 500: // Syntax error
        case 501: // Syntax error in parameters or arguments
        case 502: // Command not implemented
        case 530: // Not logged in
        case 421: // Service not available, closing control connection.
        default: return -1;
    }
}

int init_retrieve(int ctrl_socket_fd, char *path) {
    char msg[MAX_CTRL_MSG_SIZE];
    int reply_code = -1;

    snprintf(msg, sizeof msg, "retr %s\n", path);

    if (send_msg(ctrl_socket_fd, msg) == -1) {
        return -1;
    }

    if ((reply_code = read_reply_code(ctrl_socket_fd, msg)) == -1) {
        return -1;
    }

    switch (reply_code) {
        case 150: // File status okay; about to open data connection.
        case 125: // Data connection already open; transfer starting.
            return 0;

        case 110: // Restart marker reply.
        case 425: // Can't open data connection.
        case 426: // Connection closed; transfer aborted.
        case 530: // Not logged in.
        case 451: // Requested action aborted: local error in processing.
        case 450: // Requested file action not taken. File unavailable
                  // (e.g., file busy).
        case 550: //  Requested action not taken. File unavailable (e.g.,
                  //  file not found, no access).
        case 500: // Syntax error, command unrecognized. This may include
                  // errors such as command line too long.
        case 501: // Syntax error in parameters or arguments.
        case 421: // Service not available, closing
                  // control connection.
        default: return -1;
    }
}

int end_retrieve(int ctrl_socket_fd) {
    char msg[MAX_CTRL_MSG_SIZE];
    int reply_code = -1;

    if ((reply_code = read_reply_code(ctrl_socket_fd, msg)) == -1) {
        return -1;
    }

    switch (reply_code) {
        case 226: // Closing data connection. Requested file action
                  // successful (for example,file transfer or file abort)
        case 250: // Requested file action okay, completed.
            return 0;
        case 150: // File status okay; about to open data connection.
        case 125: // Data connection already open; transfer starting.
        case 110: // Restart marker reply.
        case 425: // Can't open data connection.
        case 426: // Connection closed; transfer aborted.
        case 530: // Not logged in.
        case 451: // Requested action aborted: local error in processing.
        case 450: // Requested file action not taken. File unavailable
                  // (e.g., file busy).
        case 550: //  Requested action not taken. File unavailable (e.g.,
                  //  file not found, no access).
        case 500: // Syntax error, command unrecognized. This may include
                  // errors such as command line too long.
        case 501: // Syntax error in parameters or arguments.
        case 421: // Service not available, closing control connection.
        default: return -1;
    }
}

int transfer_data(int data_socket_fd, int data_file_fd, size_t size) {
    char buf[MAX_BULK_DATA_MSG_SIZE];
    size_t total_bytes_read = 0;
    int bytes_read = -1;
    int no_bytes_written = -1;
    int total_bytes_written = 0;

    for (;;) {
        total_bytes_written = 0;
        bytes_read = -1;
        no_bytes_written = -1;

        if ((bytes_read = receive_data(data_socket_fd, MAX_BULK_DATA_MSG_SIZE,
                                       buf, false)) == -1) {
            return -1;
        }
        if (bytes_read == 0) {

            return 0;
        }

        while (total_bytes_written < bytes_read) {
            if ((no_bytes_written =
                     write(data_file_fd, buf + total_bytes_written,
                           bytes_read - total_bytes_written)) == -1) {
                perror("Write file");
                return -1;
            }

            total_bytes_written += no_bytes_written;
        }
        total_bytes_read += bytes_read;
        print_transfer_progress_bar(total_bytes_read, size);
    }
}

void logout(int ctrl_socket_fd) {
    char msg[MAX_CTRL_MSG_SIZE];

    snprintf(msg, sizeof msg, "quit\n");

    send_msg(ctrl_socket_fd, msg);
}
