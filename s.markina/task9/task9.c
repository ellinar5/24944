#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
    pid_t process_id = fork();
    switch (process_id) {
        case -1:
            perror("fork");
            exit(1);
        case 0:  
            // Дочерний процесс
            printf(" CHILD: This is a descendant process!\n");
            printf(" CHILD: My PID is %d\n", getpid());
            printf(" CHILD: PID of my parent is %d\n", getppid());
            execlp("cat", "cat", "input.txt", NULL);

            // Если произошла ошибка
            perror("execlp");
            exit(1);
        default:
            // Родительский процесс
            printf("PARENT: This is a parent process!\n");
            printf("PARENT: My PID is %d\n", getpid());
            printf("PARENT: PID of my descendant is %d\n", process_id);
            printf("PARENT: I'm waiting my descendant to call exit()...\n");
            
            // Ждем завершения дочернего процесса
            int status;
            waitpid(process_id, &status, 0);
            
            // Проверяем статус завершения
            if (WIFEXITED(status)) {
                printf("PARENT: Child exited with status %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("PARENT: Child terminated by signal %d\n", WTERMSIG(status));
            }
            
            // Эта строка выведется ПОСЛЕ завершения дочернего процесса
            printf("PARENT: Child process has finished. Parent exiting!\n");
    }

    return 0;
}