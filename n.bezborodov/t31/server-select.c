#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>   // perror
#include <stddef.h>

#define SOCKET_PATH "/tmp/uppercase_socket_nbezborodov"
#define BUF_SIZE 1024

static volatile sig_atomic_t stop_flag = 0;

static void on_signal(int sig) {
    (void)sig;
    stop_flag = 1;
}

static void safe_write_str(int fd, const char *s) {
    if (!s) return;
    (void)write(fd, s, strlen(s));
}

int main(void) {
    int server_fd = -1;
    struct sockaddr_un addr;

    /* массив клиентов: -1 = свободно */
    int client_fds[FD_SETSIZE];
    for (int i = 0; i < FD_SETSIZE; i++) client_fds[i] = -1;

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 16) < 0) {
        perror("listen");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    safe_write_str(STDOUT_FILENO, "Server started (select). Socket: " SOCKET_PATH "\n");

    char buf[BUF_SIZE];

    while (!stop_flag) {
        fd_set rfds;
        FD_ZERO(&rfds);

        int maxfd = server_fd;
        FD_SET(server_fd, &rfds);

        for (int i = 0; i < FD_SETSIZE; i++) {
            int fd = client_fds[i];
            if (fd >= 0) {
                FD_SET(fd, &rfds);
                if (fd > maxfd) maxfd = fd;
            }
        }

        int r = select(maxfd + 1, &rfds, NULL, NULL, NULL);  // блокирующий
        if (r < 0) {
            if (errno == EINTR) continue; // сигнал — просто пересоберём set
            perror("select");
            break;
        }

        /* Новые подключения */
        if (FD_ISSET(server_fd, &rfds)) {
            int cfd = accept(server_fd, NULL, NULL);
            if (cfd < 0) {
                if (errno != EINTR) perror("accept");
            } else {
                int placed = 0;
                for (int i = 0; i < FD_SETSIZE; i++) {
                    if (client_fds[i] < 0) {
                        client_fds[i] = cfd;
                        placed = 1;
                        break;
                    }
                }
                if (!placed) {
                    safe_write_str(STDERR_FILENO, "Too many clients; closing connection\n");
                    close(cfd);
                }
            }
        }

        /* Чтение от клиентов */
        for (int i = 0; i < FD_SETSIZE; i++) {
            int fd = client_fds[i];
            if (fd < 0) continue;
            if (!FD_ISSET(fd, &rfds)) continue;

            ssize_t n = read(fd, buf, sizeof(buf));
            if (n > 0) {
                for (ssize_t j = 0; j < n; j++) {
                    buf[j] = (char)toupper((unsigned char)buf[j]);
                }
                /* Пишем ровно n байт */
                ssize_t off = 0;
                while (off < n) {
                    ssize_t w = write(STDOUT_FILENO, buf + off, (size_t)(n - off));
                    if (w < 0) {
                        if (errno == EINTR) continue;
                        perror("write(stdout)");
                        break;
                    }
                    off += w;
                }
            } else {
                /* n == 0 -> клиент закрылся; n < 0 -> ошибка */
                close(fd);
                client_fds[i] = -1;
            }
        }
    }

    safe_write_str(STDOUT_FILENO, "\nServer stopping...\n");

    for (int i = 0; i < FD_SETSIZE; i++) {
        if (client_fds[i] >= 0) close(client_fds[i]);
    }
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
