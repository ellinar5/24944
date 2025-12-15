#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOCK_PATH "/tmp/uds_upper.sock"
#define BUF_SIZE 1024

int main(void) {
    int fd;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    int n;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCK_PATH);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return 1;
    }

    /* отправляем на сервер всё, что вводят в stdin */
    while ((n = read(0, buf, BUF_SIZE)) > 0) {
        write(fd, buf, n);
    }

    close(fd); /* сервер увидит EOF и завершится */
    return 0;
}
