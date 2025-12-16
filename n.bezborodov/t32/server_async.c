#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h> 

#include <sys/stropts.h>   /* I_SETSIG */
#include <sys/ioctl.h>

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

static volatile sig_atomic_t got_sig = 0;

static void on_sigpoll(int signo) {
    (void)signo;
    got_sig = 1;
}

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

static void set_nonblock(int fd) {
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl < 0) die("fcntl(F_GETFL)");
    if (fcntl(fd, F_SETFL, fl | O_NONBLOCK) < 0)
        die("fcntl(F_SETFL O_NONBLOCK)");
}

/* Включаем генерацию SIGPOLL на события ввода/ошибки/разрыва */
static void enable_sigpoll(int fd) {
    int events = S_INPUT | S_HIPRI | S_ERROR | S_HANGUP;
    if (ioctl(fd, I_SETSIG, events) < 0)
        die("ioctl(I_SETSIG)");
}

int main(void) {
    int sfd;
    struct sockaddr_un addr;

    int clients[FD_SETSIZE];
    int i;

    for (i = 0; i < FD_SETSIZE; i++) clients[i] = 0;

    signal(SIGPOLL, on_sigpoll);

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd < 0) die("socket");

    unlink(SOCK_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCK_PATH);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) die("bind");
    if (listen(sfd, 20) < 0) die("listen");

    set_nonblock(sfd);
    enable_sigpoll(sfd);

    while (1) {
        pause();                 /* ждём SIGPOLL */
        if (!got_sig) continue;
        got_sig = 0;

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
            set_nonblock(cfd);
            enable_sigpoll(cfd);
        }

        /* 2) читать данные со всех клиентов */
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

    /* не дойдём */
    close(sfd);
    unlink(SOCK_PATH);
    return 0;
}
