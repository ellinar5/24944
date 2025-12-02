#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "/tmp/uppercase_socket"

// Инициализация сокета клиента
int init_client_socket() {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Не удалось создать клиентский сокет");
        exit(EXIT_FAILURE);
    }
    return sock;
}

// Установка соединения с сервером
void establish_connection(int sock) {
    struct sockaddr_un addr;
    bzero(&addr, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Сбой подключения к серверу");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

// Передача данных на сервер
void transmit_data(int sock) {
    char *data[] = {
        "Owls solo!\n",
        "I want sleeeeeeep\n",
        "Crasy Racoon let's go!\n",
        "Bye\n"
    };
    int count = sizeof(data) / sizeof(data[0]);
    printf("Запуск передачи данных...\n");
    for (int i = 0; i < count; i++){
        size_t data_len = strlen(data[i]);
        if (write(sock, data[i], data_len) != (ssize_t)data_len){
            perror("Ошибка передачи данных");
            close(sock);
            exit(EXIT_FAILURE);
        }
        printf("Передано: %s", data[i]);
        sleep(1);
    }
}

int main() {
    int client_sock;
    printf("=== Инициализация клиентского приложения ===\n");
    client_sock = init_client_socket();
    establish_connection(client_sock);
    printf("Соединение с сервером успешно установлено!\n");
    transmit_data(client_sock);
    close(client_sock);
    printf("Клиентское приложение завершило работу\n");
    return 0;
}