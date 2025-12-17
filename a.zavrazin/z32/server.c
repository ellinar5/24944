#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <aio.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./socket"
#define MAX_CLIENTS 20000
#define BUFFER_SIZE 4096

struct client {
    int fd;
    struct aiocb cb;
    char buffer[BUFFER_SIZE];
    int id;
} clients[MAX_CLIENTS];

int server_fd;
int next_client_id = 1;

/* Получение текущего времени в формате [YYYY-MM-DD HH:MM:SS] */
void timestamp(char *buf, size_t len) {
    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", &tm);
}

void aio_completion_handler(union sigval sv) {
    struct aiocb *cb = sv.sival_ptr;
    struct client *c = NULL;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (&clients[i].cb == cb) { c = &clients[i]; break; }
    }
    if (!c || c->fd == -1) return;

    ssize_t n = aio_return(cb);
    if (n > 0) {
        char ts[64];
        timestamp(ts, sizeof(ts));

        /* перевод в верхний регистр */
        for (ssize_t j = 0; j < n; j++)
            c->buffer[j] = toupper((unsigned char)c->buffer[j]);

        /* вывод лог-сообщения с таймстемпом и client#ID */
        printf("[%s] client#%d: %.*s", ts, c->id, (int)n, c->buffer);

        aio_read(cb);  // читаем дальше
    } else {
        close(c->fd);
        c->fd = -1;
    }
}

int main() {
    unlink(SOCKET_PATH);
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 512);

    for (int i = 0; i < MAX_CLIENTS; i++) clients[i].fd = -1;

    printf("AIO-сервер запущен\n");

    while (1) {
        int fd;
        while ((fd = accept(server_fd, NULL, NULL)) != -1) {
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].fd == -1) {
                    clients[i].fd = fd;
                    clients[i].id = next_client_id++;

                    memset(&clients[i].cb, 0, sizeof(struct aiocb));
                    clients[i].cb.aio_fildes = fd;
                    clients[i].cb.aio_buf = clients[i].buffer;
                    clients[i].cb.aio_nbytes = BUFFER_SIZE;
                    clients[i].cb.aio_sigevent.sigev_notify = SIGEV_THREAD;
                    clients[i].cb.aio_sigevent.sigev_notify_function = aio_completion_handler;
                    clients[i].cb.aio_sigevent.sigev_value.sival_ptr = &clients[i].cb;

                    aio_read(&clients[i].cb);
                    break;
                }
            }
        }
        usleep(10000);
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
