#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SOCK_PATH "/tmp/uds_upper.sock"
#define BUF_SIZE 1024

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

int main(void) {
    int sfd;
    struct sockaddr_un addr;
    fd_set rfds;
    int maxfd;
    int i;

    /* clients[fd] = 1 если fd клиента активен */
    int clients[FD_SETSIZE];
    for (i = 0; i < FD_SETSIZE; i++) clients[i] = 0;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd < 0) die("socket");

    unlink(SOCK_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCK_PATH);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) die("bind");
    if (listen(sfd, 20) < 0) die("listen");

    maxfd = sfd;

    while (1) {
        FD_ZERO(&rfds);
        FD_SET(sfd, &rfds);

        for (i = 0; i <= maxfd; i++) {
            if (clients[i]) FD_SET(i, &rfds);
        }

        if (select(maxfd + 1, &rfds, NULL, NULL, NULL) < 0) {
            die("select");
        }

        /* Новый клиент? */
        if (FD_ISSET(sfd, &rfds)) {
            int cfd = accept(sfd, NULL, NULL);
            if (cfd >= 0) {
                if (cfd < FD_SETSIZE) {
                    clients[cfd] = 1;
                    if (cfd > maxfd) maxfd = cfd;
                } else {
                    /* слишком большой fd для select */
                    close(cfd);
                }
            }
        }

        /* Данные от клиентов */
        for (i = 0; i <= maxfd; i++) {
            if (clients[i] && FD_ISSET(i, &rfds)) {
                char buf[BUF_SIZE];
                int n = (int)read(i, buf, BUF_SIZE);

                if (n <= 0) {
                    /* клиент закрылся */
                    close(i);
                    clients[i] = 0;
                    continue;
                }

                /* в верхний регистр */
                for (int k = 0; k < n; k++) {
                    buf[k] = (char)toupper((unsigned char)buf[k]);
                }

                /* печать на stdout */
                write(1, buf, n);
            }
        }
    }

    /* сюда не дойдём, но по-хорошему: */
    close(sfd);
    unlink(SOCK_PATH);
    return 0;
}
