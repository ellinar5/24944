#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/time.h>

#define SOCKET_PATH "./socket"
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 10240
#define TARGET_CLIENTS 1000

int main()
{
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);
    unlink(SOCKET_PATH);
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        exit(1);
    }
    if (listen(server_fd, 128) == -1)
    {
        perror("listen");
        exit(1);
    }

    fd_set allfds, readfds;
    FD_ZERO(&allfds);
    FD_SET(server_fd, &allfds);
    int max_fd = server_fd;

    int clients[MAX_CLIENTS] = {0};
    char buf[BUFFER_SIZE];

    int total_messages = 0;
    int finished_clients = 0;
    struct timeval t_start, t_end;
    int started = 0;

    printf("select-сервер запущен\n");

    while (1)
    {
        readfds = allfds;
        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) <= 0)
            continue;

        if (FD_ISSET(server_fd, &readfds))
        {
            int fd = accept(server_fd, NULL, NULL);
            if (fd != -1)
            {
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (clients[i] == 0)
                    {
                        clients[i] = fd;
                        FD_SET(fd, &allfds);
                        if (fd > max_fd)
                            max_fd = fd;
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int fd = clients[i];
            if (fd == 0 || !FD_ISSET(fd, &readfds))
                continue;

            int n = read(fd, buf, BUFFER_SIZE);
            if (n <= 0)
            {
                close(fd);
                FD_CLR(fd, &allfds);
                clients[i] = 0;
                finished_clients++;
            }
            else
            {
                if (!started)
                {
                    gettimeofday(&t_start, NULL);
                    started = 1;
                }
                total_messages++;

                for (int j = 0; j < n; j++)
                    buf[j] = toupper(buf[j]);
                write(1, buf, n);
            }
        }

        if (started && total_messages >= TARGET_CLIENTS && finished_clients >= TARGET_CLIENTS)
        {
            gettimeofday(&t_end, NULL);
            double sec = (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_usec - t_start.tv_usec) / 1e6;
            printf("\n---RESULTS ---\n");
            printf("Total messages: %d\n", total_messages);
            printf("Total time: %.6f s\n", sec);
            break;
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
