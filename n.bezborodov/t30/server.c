#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SOCK_PATH "/tmp/uds_upper.sock"
#define BUF_SIZE 1024

int main(void) {
    int sfd, cfd;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    int n, i;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd < 0) {
        perror("socket");
        return 1;
    }

    /* если сокет-файл остался от прошлого запуска */
    unlink(SOCK_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCK_PATH);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sfd);
        return 1;
    }

    if (listen(sfd, 1) < 0) {
        perror("listen");
        close(sfd);
        unlink(SOCK_PATH);
        return 1;
    }

    cfd = accept(sfd, NULL, NULL);
    if (cfd < 0) {
        perror("accept");
        close(sfd);
        unlink(SOCK_PATH);
        return 1;
    }

    /* читаем пока клиент не закроет соединение */
    while ((n = read(cfd, buf, BUF_SIZE)) > 0) {
        for (i = 0; i < n; i++) {
            buf[i] = (char)toupper((unsigned char)buf[i]);
        }
        write(1, buf, n); /* 1 = STDOUT */
    }

    close(cfd);
    close(sfd);
    unlink(SOCK_PATH); /* убрать файл сокета */
    return 0;
}
