#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./socket"

int main()
{
    int sock_fd;
    struct sockaddr_un addr;

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        close(sock_fd);
        exit(1);
    }

    char *messages[] = {"Hello world!\n", "Test message!\n", NULL};
    for (int i = 0; messages[i] != NULL; i++)
    {
        if (write(sock_fd, messages[i], strlen(messages[i])) < 0)
        {
            perror("write");
            break;
        }
        usleep(1000);
    }

    close(sock_fd);
    return 0;
}
