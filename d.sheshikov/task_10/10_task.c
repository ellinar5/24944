#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int status;
    pid_t pid = fork();

    if (pid == 0) {
        
        execvp(argv[1], &argv[1]);
    }
    else if (pid > 0) {

        wait(&status);
    }

    return 0;
}