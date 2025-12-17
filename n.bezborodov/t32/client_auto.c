#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SOCKET_PATH "/tmp/uppercase_socket_nbezborodov"
#define BUF_SIZE 1024

int main(void)
{
    int fd;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    ssize_t n;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) { perror("socket"); exit(1); }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        exit(1);
    }

    while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        ssize_t off = 0;
        while (off < n) {
            ssize_t w = write(fd, buf + off, (size_t)(n - off));
            if (w < 0) { perror("write"); close(fd); return 1; }
            off += w;
        }
    }

    close(fd);
    return 0;
}
