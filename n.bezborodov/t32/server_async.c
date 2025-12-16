#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <aio.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>

#define SOCK_PATH "/tmp/uds_upper.sock"
#define MAX_CLIENTS 128
#define BUF_SIZE 1024
#define AIO_SIG  SIGUSR1

struct client_state {
    int fd;
    int used;
    char buf[BUF_SIZE];
    struct aiocb cb;
};

static volatile sig_atomic_t got_aio = 0;

static void on_aio(int signo) {
    (void)signo;
    got_aio = 1;
}

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

static int set_nonblock(int fd) {
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl < 0) return -1;
    if (fcntl(fd, F_SETFL, fl | O_NONBLOCK) < 0) return -1;
    return 0;
}

static void start_aio_read(struct client_state *c) {
    memset(&c->cb, 0, sizeof(c->cb));
    c->cb.aio_fildes = c->fd;
    c->cb.aio_buf = c->buf;
    c->cb.aio_nbytes = BUF_SIZE;
    c->cb.aio_offset = 0;

    c->cb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    c->cb.aio_sigevent.sigev_signo = AIO_SIG;

    /* чтобы можно было понять, какой клиент завершился */
    c->cb.aio_sigevent.sigev_value.sival_ptr = c;

    if (aio_read(&c->cb) < 0) {
        /* если не получилось — закрываем клиента */
        c->used = 0;
        close(c->fd);
    }
}

int main(void) {
    int sfd;
    struct sockaddr_un addr;
    struct client_state clients[MAX_CLIENTS];
    int i;

    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i].used = 0;
        clients[i].fd = -1;
    }

    signal(AIO_SIG, on_aio);

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd < 0) die("socket");

    unlink(SOCK_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCK_PATH);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) die("bind");
    if (listen(sfd, 20) < 0) die("listen");

    /* accept делаем неблокирующим, чтобы можно было периодически принимать новых */
    if (set_nonblock(sfd) < 0) die("fcntl(O_NONBLOCK)");

    while (1) {
        /* 1) принимаем всех новых клиентов (если есть) */
        while (1) {
            int cfd = accept(sfd, NULL, NULL);
            if (cfd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                die("accept");
            }

            /* ищем слот */
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (!clients[i].used) {
                    clients[i].used = 1;
                    clients[i].fd = cfd;
                    start_aio_read(&clients[i]);
                    break;
                }
            }
            if (i == MAX_CLIENTS) {
                /* слотов нет */
                close(cfd);
            }
        }

        /* 2) ждём сигнал о завершении AIO */
        pause();

        if (!got_aio) continue;
        got_aio = 0;

        /* 3) обработка завершённых AIO */
        for (i = 0; i < MAX_CLIENTS; i++) {
            struct client_state *c = &clients[i];
            if (!c->used) continue;

            int err = aio_error(&c->cb);
            if (err == EINPROGRESS) continue;

            int n = aio_return(&c->cb);

            if (n > 0) {
                int k;
                for (k = 0; k < n; k++) {
                    c->buf[k] = (char)toupper((unsigned char)c->buf[k]);
                }
                write(1, c->buf, n);

                /* снова читаем дальше */
                start_aio_read(c);
            } else {
                /* n == 0 -> клиент закрылся, n < 0 -> ошибка */
                c->used = 0;
                close(c->fd);
            }
        }
    }

    close(sfd);
    unlink(SOCK_PATH);
    return 0;
}
