#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(){
    pid_t pid = fork();

    if (pid < 0){
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        execlp("cat", "cat", "file-9.txt", NULL);
        perror("execlp");
        exit(1);
    } 
    
    else {
        printf("Родитель: начал выполнение\n");
        waitpid(pid, NULL, 0);
        printf("Родитель: завершил после дочернего процесса\n");
    }

    return 0;
}
