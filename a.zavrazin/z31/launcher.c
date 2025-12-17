#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

int main() {
    while (1) {
        pid_t pid = fork();
        if (pid == 0) {
            // дочерний процесс — новый клиент
            execl("./client", "./client", NULL);
            exit(1); // если execl не удался
        }
        usleep(100000);
    }
}
