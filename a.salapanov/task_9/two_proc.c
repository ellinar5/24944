
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     
#include <sys/types.h>  

int main(int argc, char *argv[])
{
    

    const char *filename = argv[1];

    printf("Родитель: запускаю программу, файл = %s\n", filename);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Дочерний процесс: сейчас выполню cat(%s)\n", filename);
        execlp("cat", "cat", filename, (char *)NULL);

        perror("execlp");
        _exit(1);
    } else {
        printf("Родитель: я не жду ребёнка, продолжаю работу\n");
        printf("Родитель: это моя последняя строка (без ожидания)\n");

        return 0;
    }
}
