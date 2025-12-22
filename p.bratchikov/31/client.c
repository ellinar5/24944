#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOCKET_PATH "./socket31"
#define BUFFER_SIZE 100

static void print_usage(const char *prog_name)
{
    printf("Usage: %s\n", prog_name);
    printf("Client program for Unix domain socket.\n");
    printf("Reads text from standard input and sends it to the server.\n");
}

int main(int argc, char *argv[])
{
    if (argc > 1 &&
        (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage(argv[0]);
        return 0;
    }

    int socket_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCKET_PATH);

    if (connect(socket_fd, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1) {
        perror("connect");
        exit(1);
    }

    ssize_t bytes_read;
    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
        write(socket_fd, buffer, bytes_read);
    }

    close(socket_fd);
    return 0;
}