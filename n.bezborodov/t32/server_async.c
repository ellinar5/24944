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

static volatile sig_atomic_t got_sigio = 0;

static void on_sigio(int signo) {
    (void)signo;
    got_sigio = 1;
}

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

static void enable_async(int fd) {
    int on;
    int pgrp;

    on = 1;
    if (ioctl(fd, FIONBIO, &on) < 0) die("ioctl(FIONBIO)");

    on = 1;
    if (ioctl(fd, FIOASYNC, &on) < 0) die("ioctl(FIOASYNC)");

    /* куда слать SIGIO */
    pgrp = (int)getpid();
    if (ioctl(fd, SIOCSPGRP, &pgrp) < 0) die("ioctl(SIOCSPGRP)");
}

int main(void) {
    int sfd;
    struct sockaddr_un addr;

    int clients[FD_SETSIZE];
    int i;

    for (i = 0; i < FD_SETSIZE; i++) clients[i] = 0;

    signal(SIGIO, on_sigio);

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd < 0) die("socket");

    unlink(SOCK_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCK_PATH);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) die("bind");
    if (listen(sfd, 20) < 0) die("listen");

    enable_async(sfd);

    while (1) {
        pause();                 /* ждём SIGIO */
        if (!got_sigio) continue;
        got_sigio = 0;

        /* 1) принять всех новых клиентов */
        while (1) {
            int cfd = accept(sfd, NULL, NULL);
            if (cfd < 0) {
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
                int n = (int)read(i, buf, BUF_SIZE);

                if (n > 0) {
                    int k;
                    for (k = 0; k < n; k++)
                        buf[k] = (char)toupper((unsigned char)buf[k]);
                    write(1, buf, n);
                    continue;
                }

                if (n == 0) {
                    close(i);
                    clients[i] = 0;
                    break;
                }

                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;

                close(i);
                clients[i] = 0;
                break;
            }
        }
    }

    return 0;
}
