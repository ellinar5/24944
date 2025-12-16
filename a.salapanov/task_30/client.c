// client.c
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/uds_echo.sock"

int main(void)
{
    int client_fd;
    struct sockaddr_un server_addr;

    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    char *messages[] = {
        "Message 1: hello, server!\n",
        "Message 2: Unix domain sockets test\n",
        "Message 3: echo me back, please\n",
        "Message 4: almost done\n",
        "Message 5: goodbye!\n"
    };

    for (int i = 0; i < 5; i++) {
        ssize_t len = strlen(messages[i]);
        if (write(client_fd, messages[i], len) != len) {
            perror("write");
            close(client_fd);
            exit(EXIT_FAILURE);
        }
        
    }

    close(client_fd);
    printf("Client finished\n");
    return 0;
}
