#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <signal.h>

#define SOCK_PATH "/tmp/uppercase_socket"
#define MAX_BUF 1024
#define MAX_CLIENTS 30

volatile sig_atomic_t stop_server = 0;

void handle_signal(int sig) {
    stop_server = 1;
}

// Создание сокета для сервера
int setup_socket() {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Не удалось создать сокет");
        exit(EXIT_FAILURE);
    }
    return sock;
}

// Настройка адреса сокета
void configure_socket(int sock) {
    struct sockaddr_un addr;
    remove(SOCK_PATH);
    bzero(&addr, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Не удалось привязать сокет");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

// Преобразование данных в верхний регистр
int handle_client(int client_sock){
    char buf[MAX_BUF];
    int n;
    
    n = read(client_sock, buf, MAX_BUF - 1);
    if (n > 0){
        buf[n] = '\0';
        char *ptr = buf;
        while(*ptr){
            *ptr = toupper((unsigned char)*ptr);
            ptr++;
        }
        printf("%s", buf);
        fflush(stdout);
        return 1;
    } 
    else if (n == 0){
        return 0;
    } 
    else{
        return 0;
    }
}

int main() {
    int server_sock, client_sock, max_sd, activity;
    fd_set readfds;
    struct sockaddr_un client_addr;
    socklen_t addr_len;
    int client_sockets[MAX_CLIENTS] = {0};
    int active_clients = 0;
    
    // Обработка сигналов
    signal(SIGINT, handle_signal);
    
    printf("=== Сервер преобразования в верхний регистр ===\n");
    
    server_sock = setup_socket();
    configure_socket(server_sock);
    
    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("Ошибка listen");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    printf("Ожидание подключений по пути: %s\n", SOCK_PATH);
    printf("Нажмите Ctrl+C для завершения работы\n");
    
    while(!stop_server){
        FD_ZERO(&readfds);
        FD_SET(server_sock, &readfds);
        max_sd = server_sock;
        
        // Считаем активных клиентов
        active_clients = 0;
        for(int i = 0; i < MAX_CLIENTS; i++){
            if(client_sockets[i] > 0){
                FD_SET(client_sockets[i], &readfds);
                if(client_sockets[i] > max_sd){
                    max_sd = client_sockets[i];
                }
                active_clients++;
            }
        }
        
        // Если нет активных клиентов и сервер завершает работу, выходим
        if (stop_server && active_clients == 0) {
            break;
        }
        
        // Устанавливаем таймаут для возможности проверки stop_server
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);
        
        if (activity < 0 && !stop_server) {
            perror("Ошибка select");
            continue;
        }
        
        if(FD_ISSET(server_sock, &readfds) && !stop_server){
            addr_len = sizeof(client_addr);
            client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
            if(client_sock < 0){
                perror("Ошибка accept");
                continue;
            }
            
            for(int i = 0; i < MAX_CLIENTS; i++){
                if(client_sockets[i] == 0){
                    client_sockets[i] = client_sock;
                    printf("Новый клиент подключен (сокет: %d)\n", client_sock);
                    fflush(stdout);
                    break;
                }
            }
        }
        
        for(int i = 0; i < MAX_CLIENTS; i++){
            if(client_sockets[i] > 0 && FD_ISSET(client_sockets[i], &readfds)){
                if(handle_client(client_sockets[i]) == 0){
                    // Клиент отключился
                    close(client_sockets[i]);
                    client_sockets[i] = 0;
                }
            }
        }
    }
    
    printf("\nЗавершение работы сервера...\n");
    
    // Закрываем все клиентские сокеты
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(client_sockets[i] > 0){
            close(client_sockets[i]);
        }
    }
    
    close(server_sock);
    remove(SOCK_PATH);
    printf("Сервер завершил работу\n");
    return 0;
}