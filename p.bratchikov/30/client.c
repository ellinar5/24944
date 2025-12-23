#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 100

static const char *unix_socket_path = "./socket30";


void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("\n");
    printf("UNIX domain socket client.\n");
    printf("Reads data from standard input and sends it to the server.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help    Show this help message and exit\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[1]);
            print_usage(argv[0]);
            return 1;
        }
    }

    char buffer[BUFFER_SIZE];
    int socket_fd;
    ssize_t bytes_read;


    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, unix_socket_path,
            sizeof(server_addr.sun_path) - 1);


    if (connect(socket_fd, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1) {
        perror("connect");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
        ssize_t bytes_written = write(socket_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            if (bytes_written >= 0) {
                fprintf(stderr, "partial write\n");
            } else {
                perror("write");
                close(socket_fd);
                exit(EXIT_FAILURE);
            }
        }
    }

    close(socket_fd);
    return 0;
}
