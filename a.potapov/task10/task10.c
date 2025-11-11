#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <команда> [аргументы...]\n", argv[0]);
        exit(1);
    }
    
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("Ошибка при создании процесса");
        exit(1);
    }
    
    if (pid == 0) {
        // Дочерний процесс
        execvp(argv[1], &argv[1]);
        perror("Ошибка при выполнении команды");
        exit(1);
    } else {
        // Родительский процесс
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Код завершён\n");
        } else {
            printf("Процесс завершился ненормально\n");
        }
    }
    
    return 0;
}