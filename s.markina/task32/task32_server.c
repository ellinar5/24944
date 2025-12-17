/* 
 * Сервер на AIO (асинхронный ввод-вывод)
 * Ключевые отличия
 * 1. Асинхронные операции: неблокирующие вызовы с колбэками
 * 2. Масштабируемость: лучше для большого числа соединений
 */
#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <aio.h>      // асинхронный ввод-вывод (вместо select/poll)
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <stdatomic.h>  // Атомарные операции

#define UNIX_SOCKET "./socket"
#define MAX_BUFFER 4096
#define MAX_CONNECTIONS 1024

// Структура контекста клиента для AIO
//  каждый клиент имеет свой контекст
typedef struct{
    int socket_fd; 
    struct aiocb aio_ctl;
    char buffer[MAX_BUFFER];     
    _Atomic int active;          
} client_context_t;

// Глобальные структуры )
client_context_t clients[MAX_CONNECTIONS]; 
int master_socket;
_Atomic int msg_counter = 0;  
struct timeval program_start, first_msg, last_msg;
_Atomic int got_first_msg = 0;
_Atomic int total_clients_connected = 0;
_Atomic int total_clients_disconnected = 0;
_Atomic int shutdown_requested = 0;

void aio_completion_handler(union sigval sigval_arg){
    // Получаем указатель на структуру управления AIO
    struct aiocb *aio_req = sigval_arg.sival_ptr;
    client_context_t *client = NULL;
    int client_index = -1;
    
    // Поиск клиента по структуре aiocb (линейный поиск)
    for (int i = 0; i < MAX_CONNECTIONS; i++){
        if (&clients[i].aio_ctl == aio_req){
            client = &clients[i];
            client_index = i;
            break;
        }
    }
    
    if (!client || client->socket_fd == -1) { return; }
    
    // Получение результата асинхронной операции
    ssize_t bytes_read = aio_return(aio_req);
    
    if (bytes_read > 0){
        // Обработка успешно прочитанных данных
        atomic_fetch_add(&msg_counter, 1); 
        struct timeval now;
        gettimeofday(&now, NULL);
        
        // Атомарная проверка и установка флага первого сообщения
        int first_msg_expected = 0;
        if (atomic_compare_exchange_strong(&got_first_msg, &first_msg_expected, 1)){
            first_msg = now;
        }
        last_msg = now;
        
        // Вычисление времени от начала работы сервера
        long sec_from_start = now.tv_sec - program_start.tv_sec;
        long usec_from_start = now.tv_usec - program_start.tv_usec;
        if (usec_from_start < 0){
            sec_from_start--;
            usec_from_start += 1000000L;
        }
        
        // Преобразование в верхний регистр
        for (int j = 0; j < bytes_read; j++) { 
            client->buffer[j] = toupper(client->buffer[j]); 
        }
        
        // Формирование и вывод результата
        char time_str[32];
        int time_len = snprintf(time_str, sizeof(time_str), 
                               "[%ld.%03ld] ", sec_from_start, usec_from_start / 1000);
        write(STDOUT_FILENO, time_str, time_len);
        write(STDOUT_FILENO, client->buffer, bytes_read);
        
        // Добавление перевода строки при необходимости
        if (bytes_read > 0 && client->buffer[bytes_read-1] != '\n') { 
            write(STDOUT_FILENO, "\n", 1); 
        }
        
        memset(&client->aio_ctl, 0, sizeof(struct aiocb));
        client->aio_ctl.aio_fildes = client->socket_fd;
        client->aio_ctl.aio_buf = client->buffer;
        client->aio_ctl.aio_nbytes = MAX_BUFFER;
        client->aio_ctl.aio_offset = 0;
        client->aio_ctl.aio_sigevent.sigev_notify = SIGEV_THREAD;  // Callback в отдельном потоке
        client->aio_ctl.aio_sigevent.sigev_notify_function = aio_completion_handler;
        client->aio_ctl.aio_sigevent.sigev_value.sival_ptr = &client->aio_ctl;
        
        // Запуск новой асинхронной операции чтения
        if (aio_read(&client->aio_ctl) == -1){
            close(client->socket_fd);
            client->socket_fd = -1;
            atomic_store(&client->active, 0);
            atomic_fetch_add(&total_clients_disconnected, 1);
        }
    }
    else if (bytes_read == 0 || (bytes_read == -1 && errno != EINPROGRESS))
    {
        // Клиент отключился или произошла ошибка
        close(client->socket_fd);
        client->socket_fd = -1;
        atomic_store(&client->active, 0);
        atomic_fetch_add(&total_clients_disconnected, 1);
        
        if (atomic_load(&total_clients_connected) > 0 && 
            atomic_load(&total_clients_disconnected) >= atomic_load(&total_clients_connected)){
            atomic_store(&shutdown_requested, 1);  // Установка флага завершения
        }
    }
}

int main(){
    for (int i = 0; i < MAX_CONNECTIONS; i++){
        clients[i].socket_fd = -1;
        atomic_store(&clients[i].active, 0);
    }
    
    gettimeofday(&program_start, NULL);
    
    master_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    int flags = fcntl(master_socket, F_GETFL, 0);
    fcntl(master_socket, F_SETFL, flags | O_NONBLOCK);
    
    // Настройка адреса и привязка сокета
    struct sockaddr_un addr = {0};
    unlink(UNIX_SOCKET);
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, UNIX_SOCKET);
    
    bind(master_socket, (struct sockaddr*)&addr, sizeof(addr));
    listen(master_socket, 128);
    
    printf("AIO Server is working (asynchronous I/O)\n");
    
    struct timeval last_activity;
    gettimeofday(&last_activity, NULL);
    int timeout_seconds = 5;  // Таймаут неактивности
    
    while (1){
        int new_client;
        while ((new_client = accept(master_socket, NULL, NULL)) != -1){
            // Поиск свободного слота для нового клиента
            int slot = -1;
            for (int i = 0; i < MAX_CONNECTIONS; i++){
                if (clients[i].socket_fd == -1){
                    slot = i;
                    break;
                }
            }
            
            if (slot == -1){
                printf("No free slots, rejecting connection\n");
                close(new_client);
                continue;
            }
            
            // Инициализация структуры клиента
            clients[slot].socket_fd = new_client;
            atomic_store(&clients[slot].active, 1);
            atomic_fetch_add(&total_clients_connected, 1);
            
            printf("New client connected. Total connected: %d\n", 
                   atomic_load(&total_clients_connected));
            
            // Настройка структуры aiocb для асинхронного чтения
            memset(&clients[slot].aio_ctl, 0, sizeof(struct aiocb));
            clients[slot].aio_ctl.aio_fildes = new_client;
            clients[slot].aio_ctl.aio_buf = clients[slot].buffer;
            clients[slot].aio_ctl.aio_nbytes = MAX_BUFFER;
            clients[slot].aio_ctl.aio_offset = 0;
            clients[slot].aio_ctl.aio_sigevent.sigev_notify = SIGEV_THREAD;  // Callback стиль
            clients[slot].aio_ctl.aio_sigevent.sigev_notify_function = aio_completion_handler;
            clients[slot].aio_ctl.aio_sigevent.sigev_value.sival_ptr = &clients[slot].aio_ctl;
            
            if (aio_read(&clients[slot].aio_ctl) == -1){
                perror("aio_read");
                close(new_client);
                clients[slot].socket_fd = -1;
                atomic_store(&clients[slot].active, 0);
                atomic_fetch_sub(&total_clients_connected, 1);
            }
            
            gettimeofday(&last_activity, NULL);
        }
        
        if (errno != EWOULDBLOCK && errno != EAGAIN) { 
            perror("accept"); 
        }
        
        int connected = atomic_load(&total_clients_connected);
        int disconnected = atomic_load(&total_clients_disconnected);
        
        if (connected > 0 && disconnected >= connected) { 
            break; 
        }
        
        struct timeval now;
        gettimeofday(&now, NULL);
        long elapsed = (now.tv_sec - last_activity.tv_sec);
        
        if (elapsed >= timeout_seconds && connected == 0 && msg_counter > 0) { 
            break; 
        }
    
        usleep(100000);
    }
    
    sleep(1);  
    
    // Вывод статистики
    long sec_diff = last_msg.tv_sec - first_msg.tv_sec;
    long usec_diff = last_msg.tv_usec - first_msg.tv_usec;
    
    if (usec_diff < 0){
        sec_diff--;
        usec_diff += 1000000L;
    }
    
    printf("\nAIO Server closed\n");
    printf("Time between first and last messages: %ld.%06ld sec\n", sec_diff, usec_diff);
    printf("Total messages processed: %d\n", atomic_load(&msg_counter));

    for (int i = 0; i < MAX_CONNECTIONS; i++){
        if (clients[i].socket_fd != -1) {
            close(clients[i].socket_fd);
            clients[i].socket_fd = -1;
        }
    }
    
    close(master_socket);
    unlink(UNIX_SOCKET);
    
    return 0;
}