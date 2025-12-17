#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#define SOCK_PATH "/tmp/uds_upper.sock"
#define BUF_SIZE  1024

#define DELAY_MS  300   /* задержка между отправками */
#define MAX_MSGS  0     /* 0 = бесконечно, иначе число сообщений */

static volatile sig_atomic_t stop = 0;

static void on_sigint(int signo) {
    (void)signo;
    stop = 1;
}

/* надёжная запись: дописывает всё, даже если write() записал частично */
static int write_all(int fd, const void *buf, size_t n) {
    const char *p = (const char *)buf;
    while (n > 0) {
        ssize_t w = write(fd, p, n);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        p += (size_t)w;
        n -= (size_t)w;
    }
    return 0;
}

int main(void) {
    int fd;
    struct sockaddr_un addr;

    signal(SIGINT, on_sigint);
    signal(SIGTERM, on_sigint);

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return 1;
    }

    pid_t pid = getpid();
    unsigned long long counter = 1;

    while (!stop) {
        char msg[BUF_SIZE];

        /* Можно сделать любую автогенерацию текста */
        int len = snprintf(msg, sizeof(msg),
                           "client pid=%ld: hello #%llu AbCdEfGhIjK!\n",
                           (long)pid, counter);

        if (len < 0) break;
        if (len > (int)sizeof(msg)) len = (int)sizeof(msg);

        if (write_all(fd, msg, (size_t)len) < 0) {
            perror("write");
            break;
        }

        counter++;
        if (MAX_MSGS > 0 && counter > (unsigned long long)MAX_MSGS) break;

        usleep((useconds_t)(DELAY_MS * 1000));
    }

    close(fd);
    return 0;
}
