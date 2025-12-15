#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/uppercase_socket"

int main()
{
    int client_fd;
    struct sockaddr_un server_addr;

    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    char *messages[] = {
        "Programming is fun!\n",
        "Unix pipes are awesome!\n", 
        "Process communication rocks!\n",
        "C language forever!\n",
        "Goodbye world!\n"
    };

    for (int i = 0; i < 5; i++)
    {
        ssize_t len = strlen(messages[i]);
        if (write(client_fd, messages[i], len) != len)
        {
            perror("write");
            close(client_fd);
            exit(EXIT_FAILURE);
        }
        sleep(1);
    }

    close(client_fd);
    printf("Client finished\n");
    return 0;
}