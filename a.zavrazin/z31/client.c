#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./socket"

int main() {
    int sock_fd;
    struct sockaddr_un addr;

    const char *messages[] = { "hello world\n", NULL };

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) { perror("socket"); exit(1); }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect"); close(sock_fd); exit(1);
    }

    // бесконечно отправляем сообщения
    while (1) {
        for (int i = 0; messages[i]; i++) {
            write(sock_fd, messages[i], strlen(messages[i]));
            usleep(500000);
        }
    }
}
