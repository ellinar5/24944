#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define SOCKET_PATH "./socket31"
#define MAX_CLIENTS 10
#define BUFFER_SIZE 100

static void print_usage(const char *prog_name)
{
    printf("Usage: %s\n", prog_name);
    printf("Server program using Unix domain socket.\n");
    printf("Accepts multiple clients, receives text, converts it to uppercase\n");
    printf("and prints the result to standard output.\n");
}

int main(int argc, char *argv[])
{
    if (argc > 1 &&
        (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage(argv[0]);
        return 0;
    }

    int server_fd;
    int client_fd;
    struct sockaddr_un server_addr;
    struct pollfd poll_fds[MAX_CLIENTS + 1];
    char buffer[BUFFER_SIZE];

    unlink(SOCKET_PATH);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("listen");
        exit(1);
    }

    poll_fds[0].fd = server_fd;
    poll_fds[0].events = POLLIN;

    for (int i = 1; i <= MAX_CLIENTS; i++)
        poll_fds[i].fd = -1;

    while (1) {
        if (poll(poll_fds, MAX_CLIENTS + 1, -1) == -1) {
            perror("poll");
            exit(1);
        }

        if (poll_fds[0].revents & POLLIN) {
            client_fd = accept(server_fd, NULL, NULL);
            if (client_fd == -1)
                continue;

            for (int i = 1; i <= MAX_CLIENTS; i++) {
                if (poll_fds[i].fd == -1) {
                    poll_fds[i].fd = client_fd;
                    poll_fds[i].events = POLLIN;
                    break;
                }
            }
        }

        for (int i = 1; i <= MAX_CLIENTS; i++) {
            if (poll_fds[i].fd == -1)
                continue;

            if (poll_fds[i].revents & POLLIN) {
                ssize_t bytes_read =
                    read(poll_fds[i].fd, buffer, sizeof(buffer));

                if (bytes_read <= 0) {
                    close(poll_fds[i].fd);
                    poll_fds[i].fd = -1;
                    continue;
                }

                for (ssize_t j = 0; j < bytes_read; j++) {
                    buffer[j] = toupper((unsigned char)buffer[j]);
                    printf("%c", buffer[j]);
                }
            }

            if (poll_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                close(poll_fds[i].fd);
                poll_fds[i].fd = -1;
            }
        }
    }
}