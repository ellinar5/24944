/* server_async.c — Solaris / UNIX domain socket server with SIGPOLL/SIGIO async I/O */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sys/select.h>   /* FD_SETSIZE */
#include <sys/filio.h>    /* FIONBIO, FIOASYNC */
#include <sys/sockio.h>   /* SIOCSPGRP */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>

#define SOCK_PATH "/tmp/uds_upper.sock"
#define BUF_SIZE  1024

static volatile sig_atomic_t got_sig = 0;

static void on_async_signal(int signo) {
    (void)signo;
    got_sig = 1;
}

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

/* Включить non-block + async уведомления для fd */
static void enable_async(int fd) {
    int on;
    int pgrp;

    on = 1;
    if (ioctl(fd, FIONBIO, &on) < 0) die("ioctl(FIONBIO)");

    on = 1;
    if (ioctl(fd, FIOASYNC, &on) < 0) die("ioctl(FIOASYNC)");

    /* куда слать SIGPOLL/SIGIO */
    pgrp = (int)getpid();
    if (ioctl(fd, SIOCSPGRP, &pgrp) < 0) die("ioctl(SIOCSPGRP)");
}

static void install_handlers(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_async_signal;
    sa.sa_flags   = SA_RESTART;      /* чтобы read/accept нормально переживали сигнал */
    sigemptyset(&sa.sa_mask);

    /* На Solaris часто прилетает SIGPOLL, а не SIGIO */
    if (sigaction(SIGPOLL, &sa, NULL) < 0) die("sigaction(SIGPOLL)");
    if (sigaction(SIGIO,   &sa, NULL) < 0) die("sigaction(SIGIO)");
}

int main(void) {
    int sfd;
    struct sockaddr_un addr;
    int clients[FD_SETSIZE];
    int i;

    for (i = 0; i < FD_SETSIZE; i++) clients[i] = 0;

    install_handlers();

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd < 0) die("socket");

    unlink(SOCK_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) die("bind");
    if (listen(sfd, 20) < 0) die("listen");

    enable_async(sfd);

    while (1) {
        /* ждём любой из сигналов async I/O */
        pause();

        if (!got_sig) continue;
        got_sig = 0;

        /* 1) принять всех новых клиентов */
        while (1) {
            int cfd = accept(sfd, NULL, NULL);
            if (cfd < 0) {
                if (errno == EINTR) continue;
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                die("accept");
            }

            if (cfd < FD_SETSIZE) {
                clients[cfd] = 1;
                enable_async(cfd);
            } else {
                close(cfd);
            }
        }

        /* 2) читать данные от всех клиентов */
        for (i = 0; i < FD_SETSIZE; i++) {
            if (!clients[i]) continue;

            while (1) {
                char buf[BUF_SIZE];
                ssize_t n = read(i, buf, sizeof(buf));

                if (n > 0) {
                    ssize_t k;
                    for (k = 0; k < n; k++)
                        buf[k] = (char)toupper((unsigned char)buf[k]);

                    /* пишем на stdout (можно обратно клиенту — по заданию как надо) */
                    (void)write(STDOUT_FILENO, buf, (size_t)n);
                    continue;
                }

                if (n == 0) {
                    /* клиент закрылся */
                    close(i);
                    clients[i] = 0;
                    break;
                }

                /* n < 0 */
                if (errno == EINTR) continue;
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;

                /* прочая ошибка — закрываем */
                close(i);
                clients[i] = 0;
                break;
            }
        }
    }

    return 0;
}
