#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>

#define SOCK_PATH "/tmp/uppercase_socket"
#define RUNTIME_SECONDS 10

int init_client_socket() {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Не удалось создать клиентский сокет");
        exit(EXIT_FAILURE);
    }
    return sock;
}

void establish_connection(int sock) {
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Сбой подключения к серверу");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    int client_sock;
    time_t start_time;
    pid_t pid = getpid();
    
    char *data[] = {
        "Owls solo!\n",
        "I want sleeeeeeep\n",
        "Crasy Racoon let's go!\n",
        "Bye\n"
    };
    int data_count = sizeof(data) / sizeof(data[0]);
    int current_index = 0;
    
    srand(time(NULL) + pid);
    
    printf("=== Клиент PID: %d ===\n", pid);
    
    client_sock = init_client_socket();
    establish_connection(client_sock);
    
    printf("Соединение с сервером успешно установлено!\n");
    printf("Клиент будет работать %d секунд\n\n", RUNTIME_SECONDS);
    
    start_time = time(NULL);
    
    while (difftime(time(NULL), start_time) < RUNTIME_SECONDS) {
        char *current_message = data[current_index];
        
        size_t data_len = strlen(current_message);
        if (write(client_sock, current_message, data_len) != (ssize_t)data_len) {
            perror("Ошибка передачи данных");
            close(client_sock);
            exit(EXIT_FAILURE);
        }
        
        printf("Отправлено: %s", current_message);
        
        current_index = (current_index + 1) % data_count;
        
        int delay_ms = 500 + (rand() % 1000);
        usleep(delay_ms * 1000);
    }
    
    printf("\nКлиент PID: %d завершил работу после %d секунд\n", pid, RUNTIME_SECONDS);
    
    close(client_sock);
    
    return 0;
}