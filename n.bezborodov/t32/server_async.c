#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#define SOCK_PATH "/tmp/uds_upper.sock"
#define BUF_SIZE  1024

static volatile sig_atomic_t got_sigio = 0;

static void on_sigio(int signo) {
    (void)signo;
    got_sigio = 1;   /* в обработчике делаем только флаг */
}

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

static void set_nonblock_async(int fd) {
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl < 0) die("fcntl(F_GETFL)");

    if (fcntl(fd, F_SETFL, fl | O_NONBLOCK | O_ASYNC) < 0)
        die("fcntl(F_SETFL O_NONBLOCK|O_ASYNC)");

    /* кто будет получать SIGIO */
    if (fcntl(fd, F_SETOWN, getpid()) < 0)
        die("fcntl(F_SETOWN)");
}

int main(void) {
    int sfd;
    struct sockaddr_un addr;

    /* таблица клиентов по fd (как в варианте с select) */
    int clients[FD_SETSIZE];
    for (int i = 0; i < FD_SETSIZE; i++) clients[i] = 0;

    /* обработчик SIGIO */
    signal(SIGIO, on_sigio);

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd < 0) die("socket");

    unlink(SOCK_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCK_PATH);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) die("bind");
    if (listen(sfd, 20) < 0) die("listen");

    /* включаем async-сигналы на слушающем сокете */
    set_nonblock_async(sfd);

    while (1) {
        /* ждём любого сигнала (в т.ч. SIGIO) */
        pause();

        if (!got_sigio) continue;
        got_sigio = 0;

        /* 1) принять всех новых клиентов */
        while (1) {
            int cfd = accept(sfd, NULL, NULL);
            if (cfd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                die("accept");
            }

            if (cfd >= FD_SETSIZE) {
                close(cfd);
                continue;
            }

            clients[cfd] = 1;

            /* включаем async на клиентском сокете тоже */
            set_nonblock_async(cfd);
        }

        /* 2) вычитать данные со всех клиентов (если есть) */
        for (int fd = 0; fd < FD_SETSIZE; fd++) {
            if (!clients[fd]) continue;

            while (1) {
                char buf[BUF_SIZE];
                int n = (int)read(fd, buf, BUF_SIZE);

                if (n > 0) {
                    for (int k = 0; k < n; k++)
                        buf[k] = (char)toupper((unsigned char)buf[k]);

                    /* печать на stdout */
                    write(1, buf, n);
                    continue;
                }

                if (n == 0) {
                    /* клиент закрыл соединение */
                    close(fd);
                    clients[fd] = 0;
                    break;
                }

                /* n < 0 */
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    /* сейчас данных нет */
                    break;
                }

                /* другая ошибка */
                close(fd);
                clients[fd] = 0;
                break;
            }
        }
    }

    /* по-хорошему (сюда не дойдём): */
    close(sfd);
    unlink(SOCK_PATH);
    return 0;
}
