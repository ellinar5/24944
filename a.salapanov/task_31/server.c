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
#include <sys/types.h>

#define SOCK_PATH "/tmp/uds_echo.sock"
#define BUF_SIZE 4096

int main(void) {
    int server_fd = -1;
    struct sockaddr_un addr;
    fd_set master_set;   // все интересующие нас дескрипторы
    fd_set read_set;     // временный набор на каждый select
    int max_fd;          // максимальный номер дескриптора
    char buf[BUF_SIZE];

    // 1. Создаём серверный сокет
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 2. Удаляем старый файл сокета, если остался
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
    if (listen(server_fd, SOMAXCONN) == -1) {
        perror("listen");
        close(server_fd);
        unlink(SOCK_PATH);
        exit(EXIT_FAILURE);
    }

    // 6. Инициализируем набор дескрипторов
    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set);
    max_fd = server_fd;

    // Основной цикл сервера
    for (;;) {
        read_set = master_set;  // select портит набор, поэтому копируем

        int ready = select(max_fd + 1, &read_set, NULL, NULL, NULL);
        if (ready == -1) {
            if (errno == EINTR)
                continue;       // сигнал — просто повторим
            perror("select");
            break;
        }

        // 1) Проверяем, не пришло ли новое подключение
        if (FD_ISSET(server_fd, &read_set)) {
            int client_fd = accept(server_fd, NULL, NULL);
            if (client_fd == -1) {
                perror("accept");
            } else {
                FD_SET(client_fd, &master_set);
                if (client_fd > max_fd)
                    max_fd = client_fd;
            }
            if (--ready <= 0)
                continue; // больше ничего не готово
        }

        // 2) Проверяем существующие клиентские сокеты
        for (int fd = 0; fd <= max_fd && ready > 0; ++fd) {
            if (fd == server_fd)
                continue;

            if (FD_ISSET(fd, &read_set)) {
                ssize_t n = read(fd, buf, BUF_SIZE);

                if (n > 0) {
                    // Переводим в верхний регистр
                    for (ssize_t i = 0; i < n; ++i) {
                        buf[i] = (char)toupper((unsigned char)buf[i]);
                    }

                    // Пишем в stdout (может перемешиваться с другими клиентами — это норм)
                    ssize_t written = 0;
                    while (written < n) {
                        ssize_t w = write(STDOUT_FILENO, buf + written, n - written);
                        if (w == -1) {
                            perror("write");
                            close(fd);
                            FD_CLR(fd, &master_set);
                            goto server_cleanup;
                        }
                        written += w;
                    }
                } else if (n == 0) {
                    // Клиент закрыл соединение
                    close(fd);
                    FD_CLR(fd, &master_set);
                    // max_fd можно не сжимать — select это переживет
                } else {
                    // Ошибка чтения
                    perror("read");
                    close(fd);
                    FD_CLR(fd, &master_set);
                }

                --ready;
            }
        }
    }

server_cleanup:
    close(server_fd);
    unlink(SOCK_PATH);
    return 0;
}
