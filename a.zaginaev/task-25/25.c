#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>

int main() {
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
        close(pipefd[1]);
        char buffer[1024];
        int n;

        while ((n = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[n] = '\0';
            for (int i = 0; i < n; i++) {
                buffer[i] = toupper(buffer[i]);
            }
            printf("Дочерний процесс: %s", buffer);
        }

        close(pipefd[0]);
        exit(0);
    } else {
        close(pipefd[0]);
        const char *text = "CVAOVKvwvvwpv ldselcv qwq\n";
        write(pipefd[1], text, strlen(text));
        close(pipefd[1]);
        waitpid(pid, NULL, 0);
        printf("Родительский процесс завершен.\n");
    }
    return 0;
}
