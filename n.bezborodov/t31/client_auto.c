#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/uppercase_socket_nbezborodov"
#define BUFFER_SIZE 1024

int main(void)
{
    int client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) { perror("socket"); exit(EXIT_FAILURE); }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
        ssize_t off = 0;
        while (off < bytes_read) {
            ssize_t w = write(client_fd, buffer + off, (size_t)(bytes_read - off));
            if (w < 0) { perror("write"); close(client_fd); return 1; }
            off += w;
        }
    }

    close(client_fd);
    return 0;
}
