#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("There are not enough arguments for this program!\n");
        exit(EXIT_FAILURE);
    }
    pid_t process = fork();
    int status;
    switch (process) {
        case -1:  // Ошибка
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:  // Дочерний процесс
            execvp(argv[1], &argv[1]);
            perror("waitpid");
            exit(1);
        default:  // Родительский процесс
            if (waitpid(process, &status, 0) == -1) {
                perror("waitpid");
                exit(1);
            }
            
            if (WIFEXITED(status)) {
                printf("Exit code of the descendant process: %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("Process terminated by signal: %d\n", WTERMSIG(status));
            }
    }
    return 0;
}