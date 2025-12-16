// server.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/time.h>

#define SOCKET_PATH "/tmp/uds_echo.sock"
#define BUF_SIZE 4096
#define MAX_CLIENTS 1024


#define IDLE_TIMEOUT_MS 20

static int set_nonblock(int fd) {
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl == -1) return -1;
    return fcntl(fd, F_SETFL, fl | O_NONBLOCK);
}

static int recalc_maxfd(int server_fd, int clients[], int n) {
    int m = server_fd;
    for (int i = 0; i < n; i++) {
        if (clients[i] != -1 && clients[i] > m) m = clients[i];
    }
    return m;
}

static void print_diff(const struct timeval *first, const struct timeval *last) {
    long sec  = (long)(last->tv_sec  - first->tv_sec);
    long usec = (long)(last->tv_usec - first->tv_usec);
    if (usec < 0) { sec--; usec += 1000000L; }
    printf("Общее время между первым и последним сообщением: %ld.%06ld сек\n", sec, usec);
}

int main(void) {
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) { perror("socket"); return 1; }

    // Чтобы accept() можно было "дренить" в цикле и не зависнуть
    if (set_nonblock(server_fd) == -1) {
        perror("fcntl(O_NONBLOCK)");
        close(server_fd);
        return 1;
    }

    unlink(SOCKET_PATH);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 128) == -1) {
        perror("listen");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    int clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) clients[i] = -1;

    fd_set allfds, readfds;
    FD_ZERO(&allfds);
    FD_SET(server_fd, &allfds);
    int maxfd = server_fd;

    char buf[BUF_SIZE];

    int any_client_seen = 0;
    int active_clients = 0;
    int total_clients = 0;

    int first_msg_received = 0;
    struct timeval first_msg_time, last_msg_time;

    printf("select-сервер запущен, сокет: %s\n", SOCKET_PATH);

    for (;;) {
        readfds = allfds;

        struct timeval tv;
        tv.tv_sec  = IDLE_TIMEOUT_MS / 1000;
        tv.tv_usec = (IDLE_TIMEOUT_MS % 1000) * 1000;

        int rc = select(maxfd + 1, &readfds, NULL, NULL, &tv);
        if (rc < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        // Таймаут: событий нет
        if (rc == 0) {
            if (any_client_seen && active_clients == 0) {
                printf("\nselect-сервер завершается\n");
                printf("Всего клиентов было: %d\n", total_clients);
                if (first_msg_received) print_diff(&first_msg_time, &last_msg_time);
                else printf("Сообщений не получено.\n");

                close(server_fd);
                unlink(SOCKET_PATH);
                return 0;
            }
            continue;
        }

        // 1) accept() — принимаем всех, кто успел подойти
        if (FD_ISSET(server_fd, &readfds)) {
            while (1) {
                int cfd = accept(server_fd, NULL, NULL);
                if (cfd == -1) {
                    if (errno == EINTR) continue;
                    if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                    perror("accept");
                    break;
                }

                any_client_seen = 1;
                total_clients++;
                active_clients++;

                int placed = 0;
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i] == -1) {
                        clients[i] = cfd;
                        placed = 1;
                        break;
                    }
                }
                if (!placed) {
                    close(cfd);
                    active_clients--;
                    continue;
                }

                FD_SET(cfd, &allfds);
                if (cfd > maxfd) maxfd = cfd;
            }
        }

        // 2) читаем клиентов
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = clients[i];
            if (fd == -1) continue;
            if (!FD_ISSET(fd, &readfds)) continue;

            ssize_t n = read(fd, buf, sizeof(buf));
            if (n <= 0) {
                close(fd);
                FD_CLR(fd, &allfds);
                clients[i] = -1;
                active_clients--;
                maxfd = recalc_maxfd(server_fd, clients, MAX_CLIENTS);
                continue;
            }

            struct timeval now;
            gettimeofday(&now, NULL);
            if (!first_msg_received) {
                first_msg_time = now;
                first_msg_received = 1;
            }
            last_msg_time = now;

            for (ssize_t j = 0; j < n; j++) {
                buf[j] = (char)toupper((unsigned char)buf[j]);
            }
            (void)write(STDOUT_FILENO, buf, (size_t)n);
        }
    }

    // аварийный выход
    close(server_fd);
    unlink(SOCKET_PATH);
    return 1;
}
