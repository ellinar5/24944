#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOCKET_PATH "/tmp/uds_socket"
#define BUF_SIZE 1024

static void print_help(const char *prog) {
    printf("Usage: %s [-h | --help]\n\n", prog);
    printf("Unix domain socket client.\n");
    printf("Connects to %s and sends text\n", SOCKET_PATH);
    printf("from standard input to the server.\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_help(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[1]);
            fprintf(stderr, "Try '%s --help'\n", argv[0]);
            return 1;
        }
    }

    int sock;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    ssize_t n;

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        exit(1);
    }

    while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        write(sock, buf, n);
    }

    close(sock);
    return 0;
}