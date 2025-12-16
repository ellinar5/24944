// server.c  (Task 32: async I/O via SIGIO, no select/poll)
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>

#define SOCKET_PATH "/tmp/uds_echo.sock"
#define BUF_SIZE 4096
#define MAX_CLIENTS 1024

#define IDLE_TIMEOUT_MS 20

static volatile sig_atomic_t g_sigio   = 0;
static volatile sig_atomic_t g_sigalrm = 0;

static void on_sigio(int s)   { (void)s; g_sigio = 1; }
static void on_sigalrm(int s) { (void)s; g_sigalrm = 1; }

static int set_async_nonblock_owner(int fd) {
    if (fcntl(fd, F_SETOWN, getpid()) == -1) return -1;

    int fl = fcntl(fd, F_GETFL, 0);
    if (fl == -1) return -1;

    if (fcntl(fd, F_SETFL, fl | O_NONBLOCK | O_ASYNC) == -1) return -1;
    return 0;
}

static void idle_timer_arm(void) {
    struct itimerval it;
    memset(&it, 0, sizeof(it));
    it.it_value.tv_sec  = IDLE_TIMEOUT_MS / 1000;
    it.it_value.tv_usec = (IDLE_TIMEOUT_MS % 1000) * 1000;
    setitimer(ITIMER_REAL, &it, NULL); // one-shot
}

static void idle_timer_disarm(void) {
    struct itimerval it;
    memset(&it, 0, sizeof(it));
    setitimer(ITIMER_REAL, &it, NULL);
}

static long diff_us(const struct timeval *a, const struct timeval *b) {
    long sec  = (long)(a->tv_sec  - b->tv_sec);
    long usec = (long)(a->tv_usec - b->tv_usec);
    return sec * 1000000L + usec;
}

// плотный массив клиентов: fds[0..n-1]
static void add_client(int fds[], int *n, int fd) {
    if (*n >= MAX_CLIENTS) { close(fd); return; }
    fds[*n] = fd;
    (*n)++;
}

static void remove_client(int fds[], int *n, int idx) {
    close(fds[idx]);
    (*n)--;
    if (idx != *n) fds[idx] = fds[*n]; // swap-with-last
}

static void print_first_last(const struct timeval *first, const struct timeval *last) {
    long us = diff_us(last, first);
    long sec = us / 1000000L;
    long usec = us % 1000000L;
    printf("Общее время между первым и последним сообщением: %ld.%06ld сек\n", sec, usec);
}

int main(void) {
    // --- handlers ---
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigio;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGIO, &sa, NULL) == -1) { perror("sigaction(SIGIO)"); return 1; }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigalrm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGALRM, &sa, NULL) == -1) { perror("sigaction(SIGALRM)"); return 1; }

    // --- server socket ---
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) { perror("socket"); return 1; }

    if (set_async_nonblock_owner(server_fd) == -1) {
        perror("fcntl(server_fd O_ASYNC/O_NONBLOCK/F_SETOWN)");
        close(server_fd);
        return 1;
    }

    unlink(SOCKET_PATH);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 128) == -1) {
        perror("listen");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    printf("SIGIO-сервер запущен, сокет: %s\n", SOCKET_PATH);

    int fds[MAX_CLIENTS];
    int nclients = 0;

    int any_client_seen = 0;
    int total_clients = 0;

    int first_msg_received = 0;
    struct timeval first_msg_time, last_msg_time;

    char buf[BUF_SIZE];

    // блокируем SIGIO/SIGALRM и ждём их через sigsuspend (без busy-loop)
    sigset_t block, oldmask;
    sigemptyset(&block);
    sigaddset(&block, SIGIO);
    sigaddset(&block, SIGALRM);
    sigprocmask(SIG_BLOCK, &block, &oldmask);

    for (;;) {
        while (!g_sigio && !g_sigalrm) {
            sigsuspend(&oldmask); // временно разблокирует сигналы
        }

        if (g_sigio) {
            g_sigio = 0;

            // есть активность → таймер "тишины" не нужен пока есть клиенты
            idle_timer_disarm();

            // 1) accept() — дреним до EAGAIN
            for (;;) {
                int cfd = accept(server_fd, NULL, NULL);
                if (cfd == -1) {
                    if (errno == EINTR) continue;
                    if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                    perror("accept");
                    break;
                }

                if (set_async_nonblock_owner(cfd) == -1) {
                    perror("fcntl(client_fd O_ASYNC/O_NONBLOCK/F_SETOWN)");
                    close(cfd);
                    continue;
                }

                any_client_seen = 1;
                total_clients++;
                add_client(fds, &nclients, cfd);
            }

            // 2) read() — пробегаем только по реально подключенным (nclients)
            for (int i = 0; i < nclients; i++) {
                int fd = fds[i];

                for (;;) {
                    ssize_t n = read(fd, buf, sizeof(buf));
                    if (n > 0) {
                        struct timeval now;
                        gettimeofday(&now, NULL);

                        if (!first_msg_received) {
                            first_msg_time = now;
                            first_msg_received = 1;
                        }
                        last_msg_time = now;

                        for (ssize_t j = 0; j < n; j++) {
                            buf[j] = (char)toupper((unsigned char)buf[j]);
                        }
                        (void)write(STDOUT_FILENO, buf, (size_t)n);
                        continue; // дочитываем до EAGAIN
                    }

                    if (n == 0) {
                        // клиент закрылся
                        remove_client(fds, &nclients, i);
                        i--; // потому что swap-with-last
                        break;
                    }

                    // n < 0
                    if (errno == EINTR) continue;
                    if (errno == EAGAIN || errno == EWOULDBLOCK) break;

                    perror("read");
                    remove_client(fds, &nclients, i);
                    i--;
                    break;
                }
            }

            // если клиентов нет и кто-то уже был — ставим one-shot таймер тишины
            if (any_client_seen && nclients == 0) {
                idle_timer_arm();
            }
        }

        if (g_sigalrm) {
            g_sigalrm = 0;

            if (any_client_seen && nclients == 0) {
                printf("\nSIGIO-сервер завершается\n");
                printf("Всего клиентов было: %d\n", total_clients);
                if (first_msg_received) print_first_last(&first_msg_time, &last_msg_time);
                else printf("Сообщений не получено.\n");

                close(server_fd);
                unlink(SOCKET_PATH);
                return 0;
            }
        }
    }
}
