#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>

#define UNIX_SOCKET "./socket"
#define MAX_BUFFER 4096
#define MAX_CONNECTIONS 1024

int main() {
    int master_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (master_socket < 0) {
        perror("socket");
        exit(1);
    }
    
    struct sockaddr_un addr = {0};
    fd_set active_set, temp_set;
    int client_fds[MAX_CONNECTIONS] = {0};
    int client_ids[MAX_CONNECTIONS] = {0};
    int next_client_id = 1;
    int max_socket = master_socket;
    char data_buffer[MAX_BUFFER];
    
    int active_clients = 0, completed_clients = 0, msg_counter = 0;
    
    struct timeval program_start, first_msg, last_msg;
    gettimeofday(&program_start, NULL);
    int got_first_msg = 0;

    // Удаляем старый сокет если существует
    unlink(UNIX_SOCKET);
    
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, UNIX_SOCKET);
    
    if (bind(master_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(master_socket);
        exit(1);
    }
    
    if (listen(master_socket, 128) < 0) {
        perror("listen");
        close(master_socket);
        exit(1);
    }

    FD_ZERO(&active_set);
    FD_SET(master_socket, &active_set);

    printf("Server is working\n");

    while (1)
    {
        temp_set = active_set;
        
        // Используем select с таймаутом для корректного завершения
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int select_result = select(max_socket + 1, &temp_set, NULL, NULL, &timeout);
        
        if (select_result < 0) {
            perror("select");
            break;
        } else if (select_result == 0) {
            // Таймаут - проверяем условия завершения
            if (active_clients > 0 && completed_clients >= active_clients) {
                break;
            }
            continue;
        }

        if (FD_ISSET(master_socket, &temp_set))
        {
            int new_client = accept(master_socket, NULL, NULL);
            if (new_client >= 0)
            {
                // Находим свободный слот
                int slot_found = 0;
                for (int i = 0; i < MAX_CONNECTIONS; i++)
                {
                    if (client_fds[i] == 0)
                    {
                        client_fds[i] = new_client;
                        client_ids[i] = next_client_id++;
                        
                        // Устанавливаем неблокирующий режим для клиента (опционально)
                        int flags = fcntl(new_client, F_GETFL, 0);
                        fcntl(new_client, F_SETFL, flags | O_NONBLOCK);
                        
                        FD_SET(new_client, &active_set);
                        if (new_client > max_socket) { 
                            max_socket = new_client; 
                        }
                        active_clients++;
                        slot_found = 1;
                        
                        printf("Client %d connected\n", client_ids[i]);
                        break;
                    }
                }
                
                if (!slot_found) {
                    printf("No free slots, rejecting connection\n");
                    close(new_client);
                }
            }
        }

        for (int i = 0; i < MAX_CONNECTIONS; i++)
        {
            int current_client = client_fds[i];
            if (current_client == 0 || !FD_ISSET(current_client, &temp_set)) { 
                continue; 
            }

            int bytes_read = read(current_client, data_buffer, MAX_BUFFER);
            if (bytes_read <= 0)
            {
                if (bytes_read < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
                    // Реальная ошибка чтения
                }
                
                printf("Client %d disconnected\n", client_ids[i]);
                close(current_client);
                FD_CLR(current_client, &active_set);
                client_fds[i] = 0;
                client_ids[i] = 0;
                completed_clients++;
                
                if (completed_clients >= active_clients && active_clients > 0)
                {
                    long sec_diff = last_msg.tv_sec - first_msg.tv_sec;
                    long usec_diff = last_msg.tv_usec - first_msg.tv_usec;
                    
                    if (usec_diff < 0)
                    {
                        sec_diff--;
                        usec_diff += 1000000L;
                    }
                    
                    printf("\nServer closed\n");
                    printf("Total messages: %d\n", msg_counter);
                    printf("Time between first and last messages: %ld.%06ld sec\n", sec_diff, usec_diff);
                    
                    close(master_socket);
                    unlink(UNIX_SOCKET);
                    return 0;
                }
            }
            else
            {
                msg_counter++;
                struct timeval now;
                gettimeofday(&now, NULL);
                
                long sec_from_start = now.tv_sec - program_start.tv_sec;
                long usec_from_start = now.tv_usec - program_start.tv_usec;
                
                if (usec_from_start < 0)
                {
                    sec_from_start--;
                    usec_from_start += 1000000L;
                }

                if (!got_first_msg)
                {
                    first_msg = now;
                    got_first_msg = 1;
                }
                last_msg = now;
                
                // Преобразование в верхний регистр
                for (int j = 0; j < bytes_read; j++) 
                { 
                    data_buffer[j] = toupper(data_buffer[j]); 
                }
                
                // Формирование вывода
                char output_buffer[MAX_BUFFER + 100];
                int output_len = snprintf(output_buffer, sizeof(output_buffer), 
                                         "[%ld.%03ld] Client <%d>: ", 
                                         sec_from_start, 
                                         usec_from_start / 1000,
                                         client_ids[i]);
                
                // Копирование данных
                int copy_len = bytes_read;
                if (output_len + copy_len >= sizeof(output_buffer))
                {
                    copy_len = sizeof(output_buffer) - output_len - 1;
                }
                
                memcpy(output_buffer + output_len, data_buffer, copy_len);
                output_len += copy_len;
                
                // Добавление перевода строки
                if (copy_len > 0 && output_buffer[output_len-1] != '\n' && output_len < sizeof(output_buffer)-1)
                {
                    output_buffer[output_len++] = '\n';
                    output_buffer[output_len] = '\0';
                }
                else if (output_len < sizeof(output_buffer))
                {
                    output_buffer[output_len] = '\0';
                }
                
                // Вывод
                write(STDOUT_FILENO, output_buffer, output_len);
            }
        }
    }

    close(master_socket);
    unlink(UNIX_SOCKET);
    return 0;
}