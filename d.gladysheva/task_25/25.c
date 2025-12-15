#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

int main()
{
    int pipefd[2];  // pipefd[0] - чтение, pipefd[1] - запись
    char buffer[256];
    ssize_t bytes_read;

    pipe(pipefd);       // создание pipe
    pid_t pid = fork(); // создание процесса

    if (pid == 0) // дочерний процесс
    {
        close(pipefd[1]); // закрытие записи
        
        while ((bytes_read = read(pipefd[0], buffer, 256)) > 0)
        {
            for (int i = 0; i < bytes_read; i++) { buffer[i] = toupper(buffer[i]); }
        
            write(STDOUT_FILENO, buffer, bytes_read);
        }
        
        close(pipefd[0]); // закрытие чтения
        exit(0);   
    }
    else // родительский процесс
    {
        close(pipefd[0]);
    
        const char *text = "Hello World!\n This is a Test String with Mixed CASE.\n End of transmission.\n";
        
        write(pipefd[1], text, strlen(text)); // передача текста по каналу
    
        close(pipefd[1]);
        
        int status;
        waitpid(pid, &status, 0); // ожидание завершения дочернего процесса
    }

    return 0;
}