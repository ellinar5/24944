#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child process: запускаем команду
        execvp(argv[1], &argv[1]);
        perror("execvp");   // выполнится только при ошибке
        return 1;
    }

    // Parent: ждём завершение дочернего процесса
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    // Определяем код завершения
    if (WIFEXITED(status)) {
        printf("Child exited with code: %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("Child terminated by signal: %d\n", WTERMSIG(status));
    } else {
        printf("Child ended in unknown way.\n");
    }

    return 0;
}
