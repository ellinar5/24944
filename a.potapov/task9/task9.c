#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }
    
    if (pid == 0) {
        execlp("cat", "cat", "/etc/passwd", NULL);
        perror("execlp failed");
        exit(1);
    } 
    else {
        int status;
        pid_t waited_pid = waitpid(pid, &status, 0);
        if (waited_pid == -1) {
            perror("waitpid failed");
        } 
        else {
            if (WIFEXITED(status)) {
                printf("Дочерний процесс завершился с кодом: %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("Дочерний процесс убит сигналом: %d\n", WTERMSIG(status));
            }
        }
        printf("РОДИТЕЛЬ: Дочерний процесс завершился, это последнее сообщение!\n");
    }
    return 0;
}