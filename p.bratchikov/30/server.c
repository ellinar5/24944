#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 100

static const char *unix_socket_path = "./socket30";


void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("\n");
    printf("UNIX domain socket server.\n");
    printf("Accepts connections and prints received data in uppercase.\n");
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
    int server_fd;
    int client_fd;
    ssize_t bytes_read;


    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, unix_socket_path,
            sizeof(server_addr.sun_path) - 1);


    unlink(unix_socket_path);

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }


    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server started. Waiting for connections...\n");

    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        while ((bytes_read = read(client_fd, buffer, sizeof(buffer))) > 0) {
            for (ssize_t i = 0; i < bytes_read; i++) {
                buffer[i] = (char)toupper((unsigned char)buffer[i]);
                putchar(buffer[i]);
            }
            fflush(stdout);
        }

        if (bytes_read == -1) {
            perror("read");
            close(client_fd);
            close(server_fd);
            exit(EXIT_FAILURE);
        } else {
            printf("\nEOF reached, closing connection\n");
            close(client_fd);
        }
    }
}