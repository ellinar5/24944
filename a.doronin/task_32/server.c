#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <aio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <errno.h>

#define SOCKET_PATH "./socket"
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 4096
#define TARGET_CLIENTS 50

struct client
{
    int fd;
    struct aiocb cb;
    char buffer[BUFFER_SIZE];
} clients[MAX_CLIENTS];

volatile int total_connected = 0;
volatile int total_finished = 0;
volatile int first_client_connected = 0;
_Atomic int active_clients = 0;

struct timespec start_time, end_time;

void aio_completion_handler(union sigval sv)
{
    struct aiocb *cb = sv.sival_ptr;
    struct client *c = NULL;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (&clients[i].cb == cb)
        {
            c = &clients[i];
            break;
        }
    }
    if (!c || c->fd == -1)
        return;

    ssize_t n = aio_return(cb);
    if (n > 0)
    {
        for (ssize_t j = 0; j < n; j++)
            c->buffer[j] = toupper((unsigned char)c->buffer[j]);
        write(STDOUT_FILENO, c->buffer, n);

        // Снова читаем асинхронно
        if (aio_read(&c->cb) < 0)
        {
            perror("aio_read");
            close(c->fd);
            c->fd = -1;
            total_finished++;
            active_clients--;
        }
    }
    else
    {
        close(c->fd);
        c->fd = -1;
        total_finished++;
        active_clients--;

        if (total_finished >= TARGET_CLIENTS)
        {
            clock_gettime(CLOCK_MONOTONIC, &end_time);
        }
    }
}

int main()
{
    int server_fd;
    struct sockaddr_un addr;

    unlink(SOCKET_PATH);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        exit(1);
    }

    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 512) < 0)
    {
        perror("listen");
        exit(1);
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
        clients[i].fd = -1;

    printf("Сервер запущен\n");

    while (total_finished < TARGET_CLIENTS)
    {
        int fd = accept(server_fd, NULL, NULL);
        if (fd >= 0)
        {
            if (total_connected >= MAX_CLIENTS)
            {
                close(fd);
                continue;
            }

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i].fd == -1)
                {
                    clients[i].fd = fd;
                    total_connected++;
                    active_clients++;

                    if (!first_client_connected)
                    {
                        clock_gettime(CLOCK_MONOTONIC, &start_time);
                        first_client_connected = 1;
                        printf("Первое подключение\n");
                    }

                    memset(&clients[i].cb, 0, sizeof(struct aiocb));
                    clients[i].cb.aio_fildes = fd;
                    clients[i].cb.aio_buf = clients[i].buffer;
                    clients[i].cb.aio_nbytes = BUFFER_SIZE;
                    clients[i].cb.aio_sigevent.sigev_notify = SIGEV_THREAD;
                    clients[i].cb.aio_sigevent.sigev_notify_function = aio_completion_handler;
                    clients[i].cb.aio_sigevent.sigev_value.sival_ptr = &clients[i].cb;

                    if (aio_read(&clients[i].cb) < 0)
                    {
                        perror("aio_read");
                        close(fd);
                        clients[i].fd = -1;
                        total_connected--;
                        active_clients--;
                    }

                    break;
                }
            }
        }
        else if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            perror("accept");
        }
    }

    double time_elapsed = (end_time.tv_sec - start_time.tv_sec) +
                          (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    printf("Сервер завершает работу\n");
    printf("Общее время между первым и последним сообщением: %.6f секунд\n", time_elapsed);

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
