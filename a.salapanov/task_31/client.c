// client.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/uds_echo.sock"
#define BUF_SIZE 4096

int main(int argc, char **argv) {
    int num = 1;
    if (argc >= 2) {
        num = atoi(argv[1]);
        if (num <= 0) num = 1;
    }

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(fd);
        return 1;
    }

    char msg[BUF_SIZE];
    int len = snprintf(msg, sizeof(msg),
                       "Hello server %d (pid=%d)\n",
                       num, (int)getpid());
    if (len < 0 || len >= (int)sizeof(msg)) {
        fprintf(stderr, "snprintf overflow\n");
        close(fd);
        return 1;
    }

    // отправляем одну строку
    if (write(fd, msg, (size_t)len) == -1) {
        perror("write");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}
