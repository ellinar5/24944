#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

int main() {
    while (1) {
        pid_t pid = fork();
        if (pid == 0) {
            execl("./client", "./client", NULL);
            exit(1);
        }
        usleep(100000); // 0.1 сек
    }
}
