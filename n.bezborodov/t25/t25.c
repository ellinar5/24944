#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <sys/types.h>  
#include <sys/wait.h>   
#include <ctype.h>      

int main(void) {
    int pipefd[2];     
    pid_t pid;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        char buf[1024];
        ssize_t n;

        close(pipefd[1]);

        while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
            for (ssize_t i = 0; i < n; i++) {
                unsigned char c = (unsigned char)buf[i];
                buf[i] = (char)toupper(c);
            }
            if (write(STDOUT_FILENO, buf, n) == -1) {
                perror("write");
                close(pipefd[0]);
                exit(1);
            }
        }

        if (n == -1) {
            perror("read");
        }

        close(pipefd[0]);
        write(STDOUT_FILENO, "\n", 1);

        exit(0);
    } else {
        char buf[1024];
        ssize_t n;

        close(pipefd[0]);

        printf("Введите текст (CTRL+D для завершения):\n");

        while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
            if (write(pipefd[1], buf, n) == -1) {
                perror("write");
                close(pipefd[1]);
                exit(1);
            }
        }

        if (n == -1) {
            perror("read");
        }

        close(pipefd[1]);

        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            exit(1);
        }

        return 0;
    }
}