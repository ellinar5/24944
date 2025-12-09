#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <errno.h>

#define SOCKET_PATH "./socket"
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 4096
#define TARGET_CLIENTS 100

struct client
{
    int fd;
    char buffer[BUFFER_SIZE];
    int bytes_in_buffer;
    int active;
    struct timespec connect_time;
    struct timespec last_msg_time;
} clients[MAX_CLIENTS];

volatile int total_connected = 0;
volatile int total_finished = 0;
struct timespec start_time, end_time;
int first_msg_received = 0;

void process_client_data(int client_index)
{
    struct client *c = &clients[client_index];

    if (c->bytes_in_buffer > 0)
    {
        if (!first_msg_received)
        {
            clock_gettime(CLOCK_MONOTONIC, &start_time);
            first_msg_received = 1;
        }

        for (int i = 0; i < c->bytes_in_buffer; i++)
            c->buffer[i] = toupper((unsigned char)c->buffer[i]);

        write(STDOUT_FILENO, c->buffer, c->bytes_in_buffer);

        clock_gettime(CLOCK_MONOTONIC, &c->last_msg_time);

        c->bytes_in_buffer = 0;
    }
}

int main()
{
    int server_fd, max_fd;
    struct sockaddr_un addr;
    fd_set read_fds, master_fds;

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
    {
        clients[i].fd = -1;
        clients[i].active = 0;
        clients[i].bytes_in_buffer = 0;
    }

    FD_ZERO(&master_fds);
    FD_SET(server_fd, &master_fds);
    max_fd = server_fd;

    printf("Сервер запущен (использует select)\n");

    while (total_finished < TARGET_CLIENTS)
    {
        read_fds = master_fds;

        int ready = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (ready < 0)
        {
            if (errno == EINTR)
                continue;
            perror("select");
            break;
        }

        if (FD_ISSET(server_fd, &read_fds))
        {
            int client_fd;
            while ((client_fd = accept(server_fd, NULL, NULL)) >= 0)
            {
                if (total_connected >= MAX_CLIENTS)
                {
                    close(client_fd);
                    continue;
                }

                int client_index = -1;
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (!clients[i].active)
                    {
                        client_index = i;
                        break;
                    }
                }

                if (client_index == -1)
                {
                    close(client_fd);
                    continue;
                }

                clients[client_index].fd = client_fd;
                clients[client_index].active = 1;
                clients[client_index].bytes_in_buffer = 0;
                clock_gettime(CLOCK_MONOTONIC, &clients[client_index].connect_time);
                memcpy(&clients[client_index].last_msg_time,
                       &clients[client_index].connect_time, sizeof(struct timespec));

                fcntl(client_fd, F_SETFL, O_NONBLOCK);

                FD_SET(client_fd, &master_fds);
                if (client_fd > max_fd)
                    max_fd = client_fd;

                total_connected++;
                printf("Принято подключение от клиента %d\n",
                       client_index);
            }

            if (client_fd < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
            {
                perror("accept");
            }
        }

        for (int i = 0; i < MAX_CLIENTS && ready > 0; i++)
        {
            if (!clients[i].active)
                continue;

            int client_fd = clients[i].fd;
            if (FD_ISSET(client_fd, &read_fds))
            {
                ssize_t bytes_read;
                while ((bytes_read = read(client_fd,
                                          clients[i].buffer + clients[i].bytes_in_buffer,
                                          BUFFER_SIZE - clients[i].bytes_in_buffer)) > 0)
                {
                    clients[i].bytes_in_buffer += bytes_read;

                    if (clients[i].bytes_in_buffer == BUFFER_SIZE ||
                        memchr(clients[i].buffer, '\n', clients[i].bytes_in_buffer))
                    {
                        process_client_data(i);
                    }
                }

                if (bytes_read == 0)
                {
                    if (clients[i].bytes_in_buffer > 0)
                        process_client_data(i);

                    printf("Клиент %d отключился\n", i);

                    close(client_fd);
                    FD_CLR(client_fd, &master_fds);
                    clients[i].active = 0;
                    clients[i].fd = -1;
                    total_finished++;

                    if (first_msg_received && total_finished >= TARGET_CLIENTS)
                    {
                        clock_gettime(CLOCK_MONOTONIC, &end_time);
                    }
                }
                else if (bytes_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    perror("read");
                    close(client_fd);
                    FD_CLR(client_fd, &master_fds);
                    clients[i].active = 0;
                    clients[i].fd = -1;
                    total_finished++;
                }

                ready--;
            }
        }
    }

    if (first_msg_received && total_finished >= TARGET_CLIENTS)
    {
        double time_elapsed = (end_time.tv_sec - start_time.tv_sec) +
                              (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

        printf("Всего клиентов: %d\n", total_finished);
        printf("Общее время между первым и последним сообщением: %.6f секунд\n", time_elapsed);
    }
    else
    {
        printf("\nСервер завершил работу, но не получил достаточное количество сообщений\n");
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].active)
        {
            close(clients[i].fd);
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}