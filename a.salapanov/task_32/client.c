// client.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>

#define SOCKET_PATH "/tmp/uds_echo.sock"
#define BUFFER_SIZE 4096

int main(void) {
    int sock_fd;
    struct sockaddr_un addr;
    char sendbuf[BUFFER_SIZE];

    // 1. Сокет
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 2. Адрес сервера
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // 3. connect
    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Уникальный идентификатор клиента: K<pid>
    char id[16];
    snprintf(id, sizeof(id), "K%d", (int)getpid());

    // 5. Сообщения 
    const char *messages[] = {
        "HELLO, SERVER!\n",
        NULL
    };

    for (int i = 0; messages[i] != NULL; i++) {
        // Собираем строку вида: "K123 HELLO, SERVER!\n"
        int len = snprintf(sendbuf, sizeof(sendbuf), "%s %s", id, messages[i]);
        if (len < 0 || len >= (int)sizeof(sendbuf)) {
            fprintf(stderr, "message too long\n");
            break;
        }

        if (write(sock_fd, sendbuf, len) == -1) {
            perror("write");
            break;
        }

        
    }

    close(sock_fd);
    return 0;
}
