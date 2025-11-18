#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

int main(){
    int pipefd[2];
    char buffer[1024];
    ssize_t bytes_read;

    if (pipe(pipefd) == -1){
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0){
        close(pipefd[1]);
        while ((bytes_read = read(pipefd[0], buffer, 1023)) > 0){
            buffer[bytes_read] = '\0';
            for (int i = 0; i < bytes_read; i++){
                buffer[i] = toupper(buffer[i]);
            }
            printf("Child process: %s\n", buffer);
            fflush(stdout);
        }
        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    }
    else{
        close(pipefd[0]);

        printf("The parent send a message: Super Owl!\n");
        write(pipefd[1], "Super Owl!", strlen("Super Owl!"));
        sleep(3);

        printf("The parent send a message: meow\n");
        write(pipefd[1], "meow", strlen("meow"));
        sleep(3);

        printf("The parent send a message: LINUx\n");
        write(pipefd[1], "LINUx", strlen("LINUx"));
        sleep(3);

        close(pipefd[1]);
        wait(NULL);
        printf("The parent process is completed.\n");
    }
    return 0;
}