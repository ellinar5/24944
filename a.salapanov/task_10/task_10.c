// run_and_status.c
#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <команда> [аргументы...]\n", argv[0]);
        return 2;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        execvp(argv[1], &argv[1]);
        perror("execvp");
        _exit(127); 
    }

    int status = 0;
    pid_t r = waitpid(pid, &status, 0);
    if (r == -1) {
        perror("waitpid");
        return 1;
    }

    int exit_code;
    if (WIFEXITED(status)) {
        exit_code = WEXITSTATUS(status);
        printf("Команда завершилась нормально, код выхода: %d\n", exit_code);
    } else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        exit_code = 128 + sig;
        printf("Команда завершена сигналом %d, сообщаем код: %d\n", sig, exit_code);
    } else {
        exit_code = 1;
        printf("Команда завершилась нестандартно (status=0x%x), код: %d\n", status, exit_code);
    }

    // Возвращаем тот же код, чтобы это было видно в оболочке (echo $?)
    return exit_code;
}
