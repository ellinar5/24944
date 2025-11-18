
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      
#include <sys/types.h>   
#include <sys/wait.h>    

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    printf("Родитель: запускаю программу, файл = %s\n", filename);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Дочерний: сейчас выполню cat(%s)\n", filename);
        execlp("cat", "cat", filename, (char *)NULL);

        perror("execlp");
        _exit(1);
    } else {
        printf("Родитель: жду, пока дочерний процесс завершит cat\n");

        int status;
        pid_t w = waitpid(pid, &status, 0);
        if (w == -1) {
            perror("waitpid");
            return 1;
        }

        if (WIFEXITED(status)) {
            printf("Родитель: дочерний процесс завершился с кодом %d\n",
                   WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Родитель: дочерний процесс был убит сигналом %d\n",
                   WTERMSIG(status));
        }

        printf("Родитель: это моя последняя строка, она печатается после cat\n");

        return 0;
    }
}
