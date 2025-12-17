#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <time.h>

#define SOCKET_PATH "./socket"
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 10240

typedef struct {
    int fd;
    int id;
} client_t;

void timestamp(char *buf, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

int main() {
    int server_fd;
    struct sockaddr_un addr;
    fd_set readfds, allfds;
    client_t clients[MAX_CLIENTS];
    int max_fd;
    int next_id = 1;
    char buf[BUFFER_SIZE];

    for (int i = 0; i < MAX_CLIENTS; i++)
        clients[i].fd = 0;

    unlink(SOCKET_PATH);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 128);

    FD_ZERO(&allfds);
    FD_SET(server_fd, &allfds);
    max_fd = server_fd;

    printf("select-сервер запущен\n");

    while (1) {
        readfds = allfds;
        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) <= 0)
            continue;

        /* Новые подключения */
        if (FD_ISSET(server_fd, &readfds)) {
            int fd = accept(server_fd, NULL, NULL);
            if (fd != -1) {
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].fd == 0) {
                        clients[i].fd = fd;
                        clients[i].id = next_id++;
                        FD_SET(fd, &allfds);
                        if (fd > max_fd) max_fd = fd;

                        char ts[64];
                        timestamp(ts, sizeof(ts));
                        printf("[%s] client#%d connected (fd=%d)\n",
                               ts, clients[i].id, fd);
                        break;
                    }
                }
            }
        }

        /* Сообщения от клиентов */
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = clients[i].fd;
            if (fd == 0 || !FD_ISSET(fd, &readfds))
                continue;

            int n = read(fd, buf, BUFFER_SIZE - 1);
            if (n > 0) {
                buf[n] = '\0';

                char ts[64];
                timestamp(ts, sizeof(ts));

                /* лог */
                printf("[%s] client#%d sent: %s", ts, clients[i].id, buf);

                /* перевод в верхний регистр */
                for (int j = 0; j < n; j++)
                    buf[j] = toupper(buf[j]);

                /* вывод результата */
                write(STDOUT_FILENO, buf, n);
            }

        }
    }
}
