#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOCKET_PATH "/tmp/uds_task31_named"
#define BUF_SIZE 256
#define MAX_CLIENTS 1024

int main() {
    int server_fd;
    struct sockaddr_un addr;

    int client_id[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++)
        client_id[i] = -1;

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    unlink(SOCKET_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 16) < 0) {
        perror("listen");
        exit(1);
    }

    fd_set master, readfds;
    FD_ZERO(&master);
    FD_SET(server_fd, &master);
    int maxfd = server_fd;

    for (;;) {
        readfds = master;

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(1);
        }

        /* Новые подключения */
        if (FD_ISSET(server_fd, &readfds)) {
            int fd = accept(server_fd, NULL, NULL);
            if (fd >= 0) {
                FD_SET(fd, &master);
                if (fd > maxfd) maxfd = fd;
            }
        }

        /* Данные от клиентов */
        for (int fd = 0; fd <= maxfd; fd++) {
            if (fd == server_fd) continue;
            if (!FD_ISSET(fd, &readfds)) continue;

            char buf[BUF_SIZE];
            ssize_t n = read(fd, buf, BUF_SIZE - 1);

            if (n <= 0) {
                close(fd);
                FD_CLR(fd, &master);
                client_id[fd] = -1;
                continue;
            }

            buf[n] = '\0';

            char *data = buf;

            /* Если ID ещё не получен — читаем его из первой строки */
            if (client_id[fd] == -1) {
                char *newline = strchr(buf, '\n');
                if (!newline) {
                    // ID ещё не полностью пришёл
                    continue;
                }

                *newline = '\0';
                client_id[fd] = atoi(buf);
                data = newline + 1;

                if (*data == '\0')
                    continue; // текста пока нет
            }

            /* Перевод текста в верхний регистр */
            for (char *p = data; *p; p++)
                *p = toupper((unsigned char)*p);

            /* Подписанный вывод */
            printf("[CLIENT %d]: %s", client_id[fd], data);
            fflush(stdout);
        }
    }
}
