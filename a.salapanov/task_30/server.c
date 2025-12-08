// server.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define SOCK_PATH "/tmp/uds_echo.sock"
#define BUF_SIZE 4096

int main(void) {
    int server_fd = -1;
    int client_fd = -1;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    ssize_t n;

    // 1. Создаём сокет
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 2. Чистим старый путь, если остался от прошлого запуска
    unlink(SOCK_PATH);

    // 3. Заполняем структуру адреса
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    // 4. bind
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 5. listen
    if (listen(server_fd, 1) == -1) {
        perror("listen");
        close(server_fd);
        unlink(SOCK_PATH);
        exit(EXIT_FAILURE);
    }

    // 6. accept (один клиент, как в условии)
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        unlink(SOCK_PATH);
        exit(EXIT_FAILURE);
    }

    // 7. Читаем данные от клиента до EOF
    while ((n = read(client_fd, buf, BUF_SIZE)) > 0) {
        // Переводим в верхний регистр
        for (ssize_t i = 0; i < n; ++i) {
            buf[i] = (char)toupper((unsigned char)buf[i]);
        }

        // Пишем в stdout
        ssize_t written = 0;
        while (written < n) {
            ssize_t w = write(STDOUT_FILENO, buf + written, n - written);
            if (w == -1) {
                perror("write");
                close(client_fd);
                close(server_fd);
                unlink(SOCK_PATH);
                exit(EXIT_FAILURE);
            }
            written += w;
        }
    }

    if (n == -1) {
        perror("read");
    }

    // 8. Чисто закрываемся
    close(client_fd);
    close(server_fd);
    unlink(SOCK_PATH);

    return 0;
}
