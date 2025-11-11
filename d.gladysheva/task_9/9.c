#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    pid_t pid = fork(); // создание процесса
    
    if (pid == 0) // дочерний процесс
    {
        execlp("cat", "cat", "input.txt", NULL); // замена образа
    }
    else // родительский процесс
    {
        FILE* file = fopen("input_for_parent.txt", "r");

        char text[256];
        char last_line[256] = "";

        while (fgets(text, sizeof(text), file))
        {
            if (last_line[0] != '\0') { printf("%s", last_line); } // вывод предыдущей строки
            strcpy(last_line, text); // сохранение текущей строки
        }

        // вывод послденей строки
        if (last_line[0] != '\0')
        {
            int status;
            waitpid(pid, &status, 0); // ожидание завершения дочернего процесса
            printf("\n%s", last_line);
        }

        fclose(file);
    }
    
    return 0;
}