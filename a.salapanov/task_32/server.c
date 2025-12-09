// server.c — асинхронный I/O через SIGIO, без select/poll
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/types.h>

#define SOCKET_PATH "/tmp/uds_echo.sock"
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 1024

static int server_fd = -1;
static int clients[MAX_CLIENTS] = {0};

static int total_clients = 0;
static int finished_clients = 0;
static int done = 0;

static struct timeval start_time;
static struct timeval first_msg_time;
static struct timeval last_msg_time;
static int first_msg_received = 0;

static void handle_sigio(int signo)
{
    if (signo != SIGIO)
        return;

    char buf[BUFFER_SIZE];

    // 1. Пробуем принять всех ожидающих клиентов
    for (;;) {
        int fd = accept(server_fd, NULL, NULL);
        if (fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // больше нет новых клиентов
            } else {
                // реальная ошибка accept — просто выходим из цикла
                break;
            }
        }

        // добавляем клиента в таблицу
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i] == 0) {
                clients[i] = fd;
                total_clients++;

                int flags = fcntl(fd, F_GETFL, 0);
                if (flags == -1) flags = 0;
                // неблокирующий и асинхронный
                fcntl(fd, F_SETFL, flags | O_NONBLOCK | O_ASYNC);
                // сигналы SIGIO будут приходить этому же процессу
                fcntl(fd, F_SETOWN, getpid());
                break;
            }
        }
    }

    // 2. Читаем данные со всех клиентов, у которых что-то есть
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        int fd = clients[i];
        if (fd == 0)
            continue;

        for (;;) {
            ssize_t n = read(fd, buf, BUFFER_SIZE);
            if (n > 0) {
                struct timeval current_time;
                gettimeofday(&current_time, NULL);

                // время с момента старта сервера
                long sec = current_time.tv_sec - start_time.tv_sec;
                long usec = current_time.tv_usec - start_time.tv_usec;
                if (usec < 0) {
                    sec--;
                    usec += 1000000L;
                }

                if (!first_msg_received) {
                    first_msg_time = current_time;
                    first_msg_received = 1;
                }
                last_msg_time = current_time;

                char time_buf[32];
                int time_len = snprintf(time_buf, sizeof(time_buf),
                                        "[%ld.%03ld] ",
                                        sec,
                                        usec / 1000);
                if (time_len > 0) {
                    write(STDOUT_FILENO, time_buf, time_len);
                }

                // делаем буквы заглавными
                for (ssize_t j = 0; j < n; ++j) {
                    buf[j] = (char)toupper((unsigned char)buf[j]);
                }
                write(STDOUT_FILENO, buf, n);

                if (n > 0 && buf[n - 1] != '\n') {
                    write(STDOUT_FILENO, "\n", 1);
                }
            } else if (n == 0) {
                // клиент закрыл соединение
                close(fd);
                clients[i] = 0;
                finished_clients++;

                if (finished_clients >= total_clients &&
                    total_clients > 0 && first_msg_received) {
                    done = 1;
                }
                break;
            } else {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // больше данных нет
                    break;
                }
                // ошибка чтения — закрываем клиента
                close(fd);
                clients[i] = 0;
                finished_clients++;

                if (finished_clients >= total_clients &&
                    total_clients > 0 && first_msg_received) {
                    done = 1;
                }
                break;
            }
        }
    }
}

int main(void)
{
    struct sockaddr_un addr;

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    unlink(SOCKET_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 128) == -1) {
        perror("listen");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    // ставим обработчик SIGIO
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigio;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGIO, &sa, NULL) == -1) {
        perror("sigaction");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    // делаем серверный сокет неблокирующим и асинхронным
    int flags = fcntl(server_fd, F_GETFL, 0);
    if (flags == -1) flags = 0;
    if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK | O_ASYNC) == -1) {
        perror("fcntl F_SETFL");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    if (fcntl(server_fd, F_SETOWN, getpid()) == -1) {
        perror("fcntl F_SETOWN");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    gettimeofday(&start_time, NULL);
    printf("async(SIGIO)-сервер запущен, сокет: %s\n", SOCKET_PATH);
    fflush(stdout);

    // ждём сигналы, пока не обработаем всех клиентов
    while (!done) {
        pause(); // просыпаемся по сигналу SIGIO
    }

    long seconds = last_msg_time.tv_sec - first_msg_time.tv_sec;
    long microseconds = last_msg_time.tv_usec - first_msg_time.tv_usec;
    if (microseconds < 0) {
        seconds--;
        microseconds += 1000000L;
    }

    printf("\nasync(SIGIO)-сервер завершается\n");
    printf("Общее время между первым и последним сообщением: %ld.%06ld сек\n",
           seconds, microseconds);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] != 0) {
            close(clients[i]);
            clients[i] = 0;
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
