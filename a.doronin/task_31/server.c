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
#define MAX_CLIENTS 1024
#define TOTAL_MESSAGES 10

int main()
{
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = {0};
    fd_set readfds, allfds;
    int clients[MAX_CLIENTS] = {0};
    int max_fd = server_fd;
    char buf[BUFFER_SIZE];
    int total_clients = 0, finished_clients = 0, message_count = 0;

    struct timeval start_time, first_msg_time, last_msg_time;
    gettimeofday(&start_time, NULL);
    int first_msg_received = 0;

    unlink(SOCKET_PATH);
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 128);

    FD_ZERO(&allfds);
    FD_SET(server_fd, &allfds);

    printf("Сервер запущен\n");

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
                        total_clients++;
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

                if (finished_clients >= total_clients && total_clients > 0)
                {

                    long seconds = last_msg_time.tv_sec - first_msg_time.tv_sec;
                    long microseconds = last_msg_time.tv_usec - first_msg_time.tv_usec;

                    if (microseconds < 0)
                    {
                        seconds--;
                        microseconds += 1000000L;
                    }

                    printf("\nСервер завершается\n");
                    printf("Общее время между первым и последним сообщением: %ld.%06ld сек\n",
                           seconds, microseconds);

                    close(server_fd);
                    unlink(SOCKET_PATH);
                    return 0;
                }
            }
            else
            {
                message_count++;
                struct timeval current_time;
                gettimeofday(&current_time, NULL);

                long seconds = current_time.tv_sec - start_time.tv_sec;
                long microseconds = current_time.tv_usec - start_time.tv_usec;

                if (microseconds < 0)
                {
                    seconds--;
                    microseconds += 1000000L;
                }

                if (!first_msg_received)
                {
                    first_msg_time = current_time;
                    first_msg_received = 1;
                }
                last_msg_time = current_time;

                char time_buf[32];
                int time_len = snprintf(time_buf, sizeof(time_buf),
                                        "[%ld.%03ld] ",
                                        seconds,
                                        microseconds / 1000);
                write(1, time_buf, time_len);

                for (int j = 0; j < n; j++)
                {
                    buf[j] = toupper(buf[j]);
                }
                write(1, buf, n);

                if (n > 0 && buf[n - 1] != '\n')
                {
                    write(1, "\n", 1);
                }
            }
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}