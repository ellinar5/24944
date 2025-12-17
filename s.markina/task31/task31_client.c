#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>

#define UNIX_SOCKET "./socket"
#define MAX_BUFFER 256

int main()
{
    int client_sock;
    struct sockaddr_un server_addr;

    // Создание сокета - используем AF_UNIX для Unix domain socket
    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, UNIX_SOCKET, sizeof(server_addr.sun_path) - 1);

    // Подключение к серверу
    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(client_sock);
        exit(1);
    }

    // Массив тестовых сообщений
    char *msg_list[] =
    {
        "Hello from the first client!\nThis is a string with mixed case.\n",
        "Second client says hello!\nChecking server operation.\n",
        "Third client is online!\nTesting the system.\n",
        "Message from the fourth client!\nEverything works perfectly.\n",
        "Fifth client checking connection!\nEnd of transmission.\n",
        "Sixth client testing!\nMixed CaSE StRinG.\n",
        "Seventh client is working!\nAnother test text.\n",
        "Eighth client connected!\nChecking letter CASE.\n",
        "Ninth client transmitting!\nFinal message.\n",
        "Tenth client finishing!\nTest completed successfully.\n",
        NULL
    };

    for (int idx = 0; msg_list[idx] != NULL; idx++)
    {
        // Отправка сообщения
        ssize_t sent = write(client_sock, msg_list[idx], strlen(msg_list[idx]));
        if (sent < 0) {
            perror("write");
            break;
        }
        
        // usleep требует unistd.h (уже добавлен)
        usleep(10000);  // 10ms задержка
    }

    // Корректное закрытие соединения
    shutdown(client_sock, SHUT_WR);
    close(client_sock);
    return 0;
}