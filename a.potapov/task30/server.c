#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "/tmp/uppercase_socket"
#define MAX_BUF 1024

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
void handle_client(int client_sock){
    char buf[MAX_BUF];
    int n;
    printf("Начата обработка клиента...\n");
    while((n = read(client_sock, buf, MAX_BUF - 1)) > 0){
        buf[n] = '\0';
        char *ptr = buf;
        while(*ptr){
            *ptr = toupper((unsigned char)*ptr);
            ptr++;
        }
        printf("Обработанные данные: %s", buf);
        fflush(stdout);
    }
    if (n < 0) {
        perror("Проблема при чтении");
    }
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_un client_addr;
    socklen_t addr_len;
    
    printf("=== Сервер преобразования в верхний регистр ===\n");
    
    server_sock = setup_socket();
    configure_socket(server_sock);
    
    if (listen(server_sock, 5) < 0) {
        perror("Ошибка listen");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    printf("Ожидание подключений по пути: %s\n", SOCK_PATH);
    addr_len = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
    if (client_sock < 0) {
        perror("Ошибка accept");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    printf("Подключение установлено!\n");
    handle_client(client_sock);
    printf("Соединение закрыто\n");
    close(client_sock);
    close(server_sock);
    remove(SOCK_PATH);
    return 0;
}