#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define UNIX_SOCKET_FILE "./socket"
#define MAX_BUFFER_LEN 256

int main() {
    int client_socket;
    struct sockaddr_un server_address;

    if ((client_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket creation error");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    strncpy(server_address.sun_path, UNIX_SOCKET_FILE, sizeof(server_address.sun_path) - 1);

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("connection error");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    char *text_messages[] =
    {
        "Hello, how ARE you doing TODAY?\n",
        "I'm FINE, thank YOU! And HOW about YOU?\n",
        "The MEETING starts at 2:30 PM in CONFERENCE room B.\n",
        "Please DON'T forget to BUY MILK and EGGS from the SUPERmarket.\n",
        "To BE, or NOT to be: that IS the QUESTION.\n",
        "ALL animals are EQUAL, but SOME animals are MORE equal than OTHERS.\n",
        "The ONLY thing we have to FEAR is fear ITSELF.\n",
        "May the FORCE be WITH you.\n",
        "MiXeD cAsE TeXt LoOkS lIkE tHiS.\n",
        "RANDOM capitalization CAN be CONFUSING to READ.\n",
        "The qUiCk BrOwN fOx JuMpS oVeR tHe LaZy DoG (alternating case).\n",
        "UPPERCASE lowercase UPPERCASE lowercase PATTERN.\n",
        NULL
    };

    for (int idx = 0; text_messages[idx] != NULL; idx++) {
        if (write(client_socket, text_messages[idx], strlen(text_messages[idx])) == -1) {
            perror("write error");
            break;
        }
        usleep(3000);
    }

    close(client_socket);
    return 0;
}