#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Not enough arguments\n");
        exit(1);
    }

    pid_t pid = fork();
    
    if (pid == -1)
    {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) // дочерний процесс
    {
        execvp(argv[1], &argv[1]);
        exit(1);
    }
    else // родительский процесс
    {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) { printf("Exit code: %d\n", WEXITSTATUS(status)); }
        
        return 0;
    }
}