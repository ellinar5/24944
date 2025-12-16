#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOCKET_PATH "/tmp/uds_task30"
#define BUF_SIZE 256

int main() {
    int fd;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    ssize_t n;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(1);
    }

    while ((n = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        write(fd, buf, n);
    }

    close(fd);
    return 0;
}
