#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOCKET_PATH "/tmp/uds_task31_named"
#define BUF_SIZE 256

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <client_id>\n", argv[0]);
        exit(1);
    }

    int fd;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];

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

    // Отправляем ID клиента первым сообщением
    snprintf(buf, sizeof(buf), "%s\n", argv[1]);
    write(fd, buf, strlen(buf));

    // Ввод текста пользователем
    while (fgets(buf, sizeof(buf), stdin)) {
        write(fd, buf, strlen(buf));
    }

    close(fd);
    return 0;
}
