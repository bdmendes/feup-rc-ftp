#include <fcntl.h>
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "message.h"
#include "network.h"
#include "states.h"
#include "url_parser.h"
#include "utils.h"

#define PATH_MAX 4096

int ctrl_socket_fd = -1;

int data_socket_fd = -1;

bool close_data_file_required = true;
int data_file_fd = -1;

static void print_usage(char *app_name) {
    fprintf(stdout,
            "Usage: %s ftp://[[user][:password]@]<host>[:port]/<url-to-file>\n",
            app_name);
}

static void close_ctrl_socket_fd() {
    if (close_socket(ctrl_socket_fd) == -1) {
        fprintf(stderr, "Failed to close control socket communications\n");
    }
    if (close(ctrl_socket_fd) == -1) {
        perror("Failed to close control socket FD");
    }
}

static void close_data_socket_fd() {
    if (close_socket(data_socket_fd) == -1) {
        fprintf(stderr, "Failed to close data socket communications\n");
    }
    if (close(data_socket_fd) == -1) {
        perror("Failed to close data socket FD");
    }
}

static void close_data_file_fd() {
    if (close_data_file_required) {
        if (close(data_file_fd) == -1) {
            perror("Failed to close data file FD");
        }
    }
}

static void close_delete_data_file(char *data_file_path) {
    if (close(data_file_fd) == -1) {
        perror("Failed to close data file FD");
    }

    close_data_file_required = false;

    if (unlink(data_file_path) == -1) {
        perror("Removing created file");
    }
}

void get_resource_name(char *server_rsrc_path, int file_name_size,
                       char *file_name) {
    strncpy(file_name, strrchr(server_rsrc_path, '/') + 1, file_name_size);
}

int create_open_data_file(char *file_name, int file_path_size,
                          char *file_path) {
    /* Create and open file for incoming data*/
    char file_path_[PATH_MAX];
    snprintf(file_path_, PATH_MAX, "./%.*s",
             (int)(strnlen(file_name, PATH_MAX)), file_name);

    for (int n = 1;; n++) {
        if (access(file_path_, F_OK) == 0) {
            snprintf(file_path_, PATH_MAX, "./%d-%.*s", n,
                     ((int)(strnlen(file_name, PATH_MAX))), file_name);
        } else {
            if (n > 1) {
                strncpy(file_name, strrchr(file_path_, '/') + 1, PATH_MAX / 4);
                fprintf(stdout,
                        "File name already exists, creating new file: %s\n",
                        file_name);
            }
            break;
        }
    }

    if (file_path != NULL) {
        strncpy(file_path, file_path_, file_path_size);
    }

    int fd = -1;
    /* Open new file with received file name */
    if ((fd = open(file_path_, O_WRONLY | O_CREAT | O_TRUNC,
                   S_IRWXU | S_IRWXG | S_IRWXO)) == -1) {
        perror("Open file for writing");
        return -1;
    }

    return fd;
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    if (argc != 2) {
        print_usage(argv[0]);
        return -1;
    }

    if (validate_ftp_url(argv[1]) != 0) {
        print_usage(argv[0]);
        return -1;
    }

    struct con_info con_info;
    if (parse_url_con_info(argv[argc - 1], &con_info) != 0) {
        print_usage(argv[0]);
        return -1;
    }

    printf("%s, %s, %s, %s, %s\n", con_info.addr, con_info.rsrc, con_info.user,
           con_info.pass, con_info.port);

    if (strnlen(con_info.rsrc, sizeof con_info.rsrc) < 1) {
        fprintf(stdout, "No file specified\n");
        return -1;
    }

    /* Open data control socket */
    int ctrl_socket_family = -1;
    if ((ctrl_socket_fd = open_connect_socket(con_info.addr, con_info.port,
                                              &ctrl_socket_family)) == -1) {
        return -1;
    }

    /* Register atexit function related to control socket FD */
    if (atexit(close_ctrl_socket_fd) != 0) {
        fprintf(stderr, "Cannot register close_ctrl_socket_fd to run atexit\n");
        if (close(ctrl_socket_fd) != -1) {
            perror("Close control socket FD");
        }

        return -1;
    }

    /* Login in the server */
    int ret = 0;
    if ((ret = login(ctrl_socket_fd, con_info.user, con_info.pass)) != 0) {
        if (ret == -2) {
            fprintf(stdout, "Unrecognized user or password credentials\n");
        }
        if (ret == -3) {
            fprintf(stdout, "Unsupported login with this application\n");
        }
        return -1;
    }

    /* Query if it is regular file and get size*/
    size_t size = 0;
    if ((ret = get_file_size(ctrl_socket_fd, con_info.rsrc, &size)) < 0) {
        if (ret == -2) {
            fprintf(stderr, "Error: Not Found\n");
        }
        if (ret == -3) {
            fprintf(stderr, "Error: Requested directory\n");
        }

        return -1;
    }

    char file_name[PATH_MAX];
    get_resource_name(con_info.rsrc, PATH_MAX, file_name);

    printf("Downloading: %s, %ld bytes\n", file_name, size);

    /* Set passive mode and open data socket */
    char pasv_addr[MAX_ADDRESS_SIZE];
    if ((ret = set_pasv_mode(ctrl_socket_fd, ctrl_socket_family, pasv_addr)) ==
        -1) {
        return -1;
    }

    bool is_ipv4 = (ret == 0);

    if (is_ipv4) {
        char buf[MAX_ADDRESS_SIZE];
        char ip[MAX_URL_LENGTH];
        char port[MAX_PORT_LENGTH];
        strncpy(buf, pasv_addr, MAX_ADDRESS_SIZE);
        snprintf(ip, sizeof ip, "%s", strtok(buf, ":"));
        snprintf(port, sizeof port, "%s", strtok(NULL, ":"));

        if ((data_socket_fd = open_connect_socket(ip, port, NULL)) == -1) {
            return -1;
        }
    } else {
        if ((data_socket_fd =
                 open_connect_socket(con_info.addr, pasv_addr, NULL)) == -1) {
            return -1;
        }
    }

    /* Register atexit function related to data socket FD */
    if (atexit(close_data_socket_fd) != 0) {
        fprintf(stderr, "Cannot register close_data_socket_fd to run atexit\n");
        if (close(data_socket_fd) != -1) {
            perror("Close data socket FD");
        }
        return -1;
    }

    /* Create and open file for incoming data*/
    char file_path[PATH_MAX];
    if ((data_file_fd =
             create_open_data_file(file_name, PATH_MAX, file_path)) == -1) {
        return -1;
    }
    if (atexit(close_data_file_fd) != 0) {
        fprintf(stderr, "Cannot register close_data_file_fd to run atexit\n");
        close_delete_data_file(file_path);
        return -1;
    }

    /* Initiate tranfer */
    if (init_retrieve(ctrl_socket_fd, con_info.rsrc) == -1) {
        return -1;
    }

    /* Read from socket and write to file */
    struct timespec start_time, end_time;
    if (clock_gettime(CLOCK_MONOTONIC, &start_time) == -1) {
        perror("Clock get start time");
    }
    if (transfer_data(data_socket_fd, data_file_fd, size) == -1) {
        return -1;
    }
    if (clock_gettime(CLOCK_MONOTONIC, &end_time) == -1) {
        perror("Clock get end time");
    }

    /* End transfer */
    if (end_retrieve(ctrl_socket_fd) == -1) {
        return -1;
    }

    /* Logout */
    logout(ctrl_socket_fd);

    struct stat st;
    if (stat(file_path, &st) == -1) {
        perror("Stat");
    } else {
        double elapsed_secs = elapsed_seconds(&start_time, &end_time);
        double kbs = ((double)st.st_size / 1000) / elapsed_secs;
        printf("Elapsed time: %.2fs\n", elapsed_secs);
        printf("Average speed: %.2fKB/s\n", kbs);
    }

    return 0;
}
