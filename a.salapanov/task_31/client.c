// client.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define SOCK_PATH "/tmp/uds_echo.sock"
#define BUF_SIZE 4096

int main(void) {
    int sock_fd = -1;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    ssize_t n;

    // 1. Создаём сокет
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 2. Адрес сервера
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    // 3. Подключаемся к серверу
    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Читаем с stdin и отправляем на сервер
    while ((n = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        ssize_t sent = 0;
        while (sent < n) {
            ssize_t s = write(sock_fd, buf + sent, n - sent);
            if (s == -1) {
                perror("write");
                close(sock_fd);
                exit(EXIT_FAILURE);
            }
            sent += s;
        }
    }

    if (n == -1) {
        perror("read");
    }

    // 5. Закрываем соединение
    close(sock_fd);
    return 0;
}
