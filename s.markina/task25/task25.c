#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

int main(){
    int pipefd[2]; 
    char buffer[256];
    ssize_t bytes_read;

    pipe(pipefd);  // создаем pipe
    pid_t pid = fork(); // создаем дочерний процесс

    if (pid == 0){
        close(pipefd[1]);
        
        while ((bytes_read = read(pipefd[0], buffer, 256)) > 0){
            // каждый символ в верхний регистр
            for (int i = 0; i < bytes_read; i++) { buffer[i] = toupper(buffer[i]); }
        
            write(STDOUT_FILENO, buffer, bytes_read);
        }
        
        close(pipefd[0]); 
        exit(0);   
    }
    else{
        close(pipefd[0]);
    
        const char *text = "abababa\n qqqqqqqqqqq \n End.\n";
        
        write(pipefd[1], text, strlen(text));
    
        close(pipefd[1]);
        
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}