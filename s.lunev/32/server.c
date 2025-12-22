/*
============================================================================
Name        : server.c
Author      : Sam Lunev
Version     : .0
Copyright   : All rights reserved
Description : OSLrCPT32
============================================================================
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <time.h>
#include <aio.h>
#include <fcntl.h>
#include <ctype.h>

#define SOCKET_PATH "/tmp/uds_socket"
#define BUFFER_SIZE 1024

void to_uppercase(char *str) {
    while (*str) {
        if (islower(*str)) {
            *str = toupper((unsigned char)*str);
        }
        str++;
    }
}

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        perror("recv failed");
        close(client_fd);
        return;
    }

    buffer[bytes_received] = '\0';

    to_uppercase(buffer);

    time_t current_time;
    time(&current_time);
    printf("Received at: %s", ctime(&current_time));
    printf("Message: %s\n", buffer);

    close(client_fd);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr, client_addr;
    socklen_t addr_len;

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen failed");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on %s\n", SOCKET_PATH);

    while (1) {
        addr_len = sizeof(client_addr);

        if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len)) == -1) {
            perror("accept failed");
            close(server_fd);
            unlink(SOCKET_PATH);
            exit(EXIT_FAILURE);
        }

        handle_client(client_fd);
    }

    close(server_fd);
    return 0;
}
