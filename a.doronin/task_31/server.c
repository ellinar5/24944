#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define SOCK_PATH "/tmp/uppercase_socket"
#define MAX_BUF 1024
#define MAX_CLIENTS 50

volatile sig_atomic_t stop_server = 0;

time_t first_message_time = 0;
time_t last_message_time = 0;
int total_messages_received = 0;

void handle_signal(int sig)
{
    stop_server = 1;
}

int setup_socket()
{
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Не удалось создать сокет");
        exit(EXIT_FAILURE);
    }
    return sock;
}

void configure_socket(int sock)
{
    struct sockaddr_un addr;
    remove(SOCK_PATH);
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Не удалось привязать сокет");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

void update_message_times()
{
    time_t current_time = time(NULL);

    if (first_message_time == 0)
    {
        first_message_time = current_time;
    }

    last_message_time = current_time;
    total_messages_received++;
}

int handle_client_nonblock(int client_sock, int client_num)
{
    char buf[MAX_BUF];
    int n;

    n = recv(client_sock, buf, MAX_BUF - 1, MSG_DONTWAIT);

    if (n > 0)
    {
        buf[n] = '\0';
        update_message_times();

        time_t now;
        struct tm *timeinfo;
        char time_str[50];

        time(&now);
        timeinfo = localtime(&now);
        strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);

        char *ptr = buf;
        while (*ptr)
        {
            *ptr = toupper((unsigned char)*ptr);
            ptr++;
        }

        printf("[%s] Клиент %d: %s", time_str, client_num, buf);
        fflush(stdout);
        return 1;
    }
    else if (n == 0)
    {
        time_t now;
        struct tm *timeinfo;
        char time_str[50];

        time(&now);
        timeinfo = localtime(&now);
        strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);

        printf("[%s] Клиент %d отключился\n", time_str, client_num);
        return 0;
    }
    else if (n < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return 1;
        }
        else
        {
            perror("Ошибка recv");
            return 0;
        }
    }
    return 1;
}

void print_statistics()
{
    if (total_messages_received > 0 && first_message_time > 0 && last_message_time > 0)
    {
        char first_time_str[50], last_time_str[50];
        struct tm *timeinfo;
        timeinfo = localtime(&first_message_time);
        strftime(first_time_str, sizeof(first_time_str), "%H:%M:%S", timeinfo);
        timeinfo = localtime(&last_message_time);
        strftime(last_time_str, sizeof(last_time_str), "%H:%M:%S", timeinfo);
        double duration = difftime(last_message_time, first_message_time);
        printf("Общее время приема сообщений: %.1f секунд\n", duration);
    }
}

int main()
{
    int server_sock, client_sock, max_sd, activity;
    fd_set readfds;
    struct sockaddr_un client_addr;
    socklen_t addr_len;
    int client_sockets[MAX_CLIENTS] = {0};
    int active_clients = 0;
    int total_clients_connected = 0;
    first_message_time = 0;
    last_message_time = 0;
    total_messages_received = 0;
    signal(SIGINT, handle_signal);
    printf("Сервер запущен\n");
    server_sock = setup_socket();
    configure_socket(server_sock);
    if (listen(server_sock, MAX_CLIENTS) < 0)
    {
        perror("Ошибка listen");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    while (!stop_server)
    {
        FD_ZERO(&readfds);
        FD_SET(server_sock, &readfds);
        max_sd = server_sock;
        active_clients = 0;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_sockets[i] > 0)
            {
                FD_SET(client_sockets[i], &readfds);
                if (client_sockets[i] > max_sd)
                {
                    max_sd = client_sockets[i];
                }
                active_clients++;
            }
        }
        if (stop_server && active_clients == 0)
        {
            break;
        }
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);
        if (activity < 0 && !stop_server)
        {
            perror("Ошибка select");
            continue;
        }
        if (FD_ISSET(server_sock, &readfds) && !stop_server)
        {
            addr_len = sizeof(client_addr);
            client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
            if (client_sock < 0)
            {
                perror("Ошибка accept");
                continue;
            }
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = client_sock;
                    total_clients_connected++;
                    time_t now;
                    struct tm *timeinfo;
                    char time_str[50];

                    time(&now);
                    timeinfo = localtime(&now);
                    strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);

                    printf("[%s] Новый клиент подключен (всего клиентов: %d)\n",
                           time_str, total_clients_connected);
                    fflush(stdout);
                    break;
                }
            }
        }
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_sockets[i] > 0 && FD_ISSET(client_sockets[i], &readfds))
            {
                if (handle_client_nonblock(client_sockets[i], i) == 0)
                {
                    close(client_sockets[i]);
                    client_sockets[i] = 0;
                    active_clients--;
                    time_t now;
                    struct tm *timeinfo;
                    char time_str[50];

                    time(&now);
                    timeinfo = localtime(&now);
                    strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);

                    printf("[%s] Осталось активных клиентов: %d\n",
                           time_str, active_clients);

                    if (active_clients == 0 && total_messages_received > 0)
                    {
                        printf("\n[%s] Все клиенты завершили работу\n", time_str);
                        print_statistics();
                    }
                }
            }
        }
    }

    time_t now;
    struct tm *timeinfo;
    char time_str[50];

    time(&now);
    timeinfo = localtime(&now);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);

    printf("\n[%s] Завершение работы сервера...\n", time_str);

    if (total_messages_received > 0)
    {
        print_statistics();
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_sockets[i] > 0)
        {
            close(client_sockets[i]);
        }
    }

    close(server_sock);
    remove(SOCK_PATH);
    printf("[%s] Сервер завершил работу\n", time_str);
    printf("Всего обработано клиентов: %d\n", total_clients_connected);

    return 0;
}
