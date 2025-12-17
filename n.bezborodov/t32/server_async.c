#define _POSIX_C_SOURCE 200112L
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <aio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SOCKET_PATH "/tmp/uppercase_socket_nbezborodov"
#define MAX_CLIENTS 64
#define READ_SIZE 1

/* событие в pipe: либо "accept needed", либо "aio done for slot" */
#define EVT_ACCEPT  0xFFFFu

struct client {
    int fd;
    int active;
    struct aiocb cb;
    unsigned char byte;
};

static struct client clients[MAX_CLIENTS];
static int listen_fd = -1;
static int evt_pipe[2] = {-1, -1};

static unsigned char to_upper_ascii(unsigned char c) {
    if (c >= 'a' && c <= 'z') return (unsigned char)(c - ('a' - 'A'));
    return c;
}

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

/* signal handler: только write() в pipe (async-signal-safe) */
static void push_event(unsigned short code) {
    /* пишем 2 байта; если pipe переполнен — просто теряем событие */
    (void)write(evt_pipe[1], &code, sizeof(code));
}

static void on_sigio(int sig) {
    (void)sig;
    push_event(EVT_ACCEPT);
}

static void on_aio_done(int sig, siginfo_t *si, void *uap) {
    (void)sig; (void)uap;
    int slot = si->si_value.sival_int;
    if (slot >= 0 && slot < MAX_CLIENTS) {
        push_event((unsigned short)slot);
    }
}

static void cleanup(int sig) {
    (void)sig;
    if (listen_fd != -1) close(listen_fd);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            aio_cancel(clients[i].fd, NULL);
            close(clients[i].fd);
            clients[i].active = 0;
            clients[i].fd = -1;
        }
    }
    if (evt_pipe[0] != -1) close(evt_pipe[0]);
    if (evt_pipe[1] != -1) close(evt_pipe[1]);
    unlink(SOCKET_PATH);
    _exit(0);
}

static void arm_aio_read(int slot) {
    memset(&clients[slot].cb, 0, sizeof(clients[slot].cb));
    clients[slot].cb.aio_fildes = clients[slot].fd;
    clients[slot].cb.aio_buf    = &clients[slot].byte;
    clients[slot].cb.aio_nbytes = READ_SIZE;

    clients[slot].cb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    clients[slot].cb.aio_sigevent.sigev_signo  = SIGRTMIN;
    clients[slot].cb.aio_sigevent.sigev_value.sival_int = slot;

    if (aio_read(&clients[slot].cb) == -1) {
        /* клиент мог уже закрыться */
        close(clients[slot].fd);
        clients[slot].fd = -1;
        clients[slot].active = 0;
    }
}

static void accept_all_pending(void) {
    for (;;) {
        int cfd = accept(listen_fd, NULL, NULL);
        if (cfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            if (errno == EINTR) continue;
            perror("accept");
            return;
        }

        int slot = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i].active) { slot = i; break; }
        }

        if (slot < 0) {
            close(cfd);
            continue;
        }

        clients[slot].fd = cfd;
        clients[slot].active = 1;

        /* стартуем первый aio_read на 1 байт */
        arm_aio_read(slot);
    }
}

static void handle_aio_slot(int slot) {
    if (slot < 0 || slot >= MAX_CLIENTS) return;
    if (!clients[slot].active) return;

    int err = aio_error(&clients[slot].cb);
    if (err == EINPROGRESS) return;

    ssize_t n = aio_return(&clients[slot].cb);

    if (n == 1) {
        unsigned char out = to_upper_ascii(clients[slot].byte);
        (void)write(STDOUT_FILENO, &out, 1);
        arm_aio_read(slot);
    } else {
        /* n == 0 => EOF; n < 0 => ошибка */
        close(clients[slot].fd);
        clients[slot].fd = -1;
        clients[slot].active = 0;
    }
}

int main(void) {
    struct sockaddr_un addr;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].active = 0;
    }

    if (pipe(evt_pipe) == -1) die("pipe");

    /* pipe лучше сделать неблокирующим, чтобы handler не завис */
    int flags = fcntl(evt_pipe[1], F_GETFL, 0);
    (void)fcntl(evt_pipe[1], F_SETFL, flags | O_NONBLOCK);

    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    /* обработчик SIGIO (новые подключения) */
    struct sigaction sa_io;
    memset(&sa_io, 0, sizeof(sa_io));
    sa_io.sa_handler = on_sigio;
    sigemptyset(&sa_io.sa_mask);
    sa_io.sa_flags = SA_RESTART;
    if (sigaction(SIGIO, &sa_io, NULL) == -1) die("sigaction(SIGIO)");

    /* обработчик AIO completion (realtime signal) */
    struct sigaction sa_aio;
    memset(&sa_aio, 0, sizeof(sa_aio));
    sa_aio.sa_sigaction = on_aio_done;
    sigemptyset(&sa_aio.sa_mask);
    sa_aio.sa_flags = SA_SIGINFO | SA_RESTART;
    if (sigaction(SIGRTMIN, &sa_aio, NULL) == -1) die("sigaction(SIGRTMIN)");

    listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd < 0) die("socket");

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCKET_PATH);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) die("bind");
    if (listen(listen_fd, 16) < 0) die("listen");

    /* включаем SIGIO на listen_fd */
    (void)fcntl(listen_fd, F_SETOWN, getpid());
    int lf = fcntl(listen_fd, F_GETFL, 0);
    (void)fcntl(listen_fd, F_SETFL, lf | O_NONBLOCK | O_ASYNC);

    const char *msg = "AIO+signals server running. Socket: " SOCKET_PATH "\n";
    (void)write(STDOUT_FILENO, msg, strlen(msg));

    /* на случай если кто-то уже коннектится сразу */
    accept_all_pending();

    /* основной цикл: блокируемся на pipe событий */
    for (;;) {
        unsigned short code;
        ssize_t r = read(evt_pipe[0], &code, sizeof(code));
        if (r < 0) {
            if (errno == EINTR) continue;
            die("read(evt_pipe)");
        }
        if (r == 0) continue;

        if (code == EVT_ACCEPT) {
            accept_all_pending();
        } else {
            handle_aio_slot((int)code);
        }
    }
}
