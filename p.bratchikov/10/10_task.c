#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) 
{

    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) 
    {
        printf("\nThis program forks a child process to execute a command passed as arguments.\n");
        printf("\nUsage: %s <command> [args...]\n", argv[0]);
        return 0;
    }

    if (argc == 1) 
    {
        fprintf(stderr, "\nError: No command provided. Use --help for usage.\n");
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1) 
    {
        fprintf(stderr, "\nfork() error: %s\n", strerror(errno));
        return 1;
    }

    if (pid == 0) 
    {
        execvp(argv[1], argv + 1);
        fprintf(stderr, "\nexecvp() error: %s\n", strerror(errno));
        return 1;
    }

    else 
    {
        
        int stat_loc;

        if (wait(&stat_loc) == -1) 
        {
            fprintf(stderr, "\nwait() error: %s\n", strerror(errno));
            return 1;
        }

        if (WIFEXITED(stat_loc)) 
        {
            printf("\nChild process (pid: %d) finished with exit code %d\n", pid, WEXITSTATUS(stat_loc));
        }

        else if (WIFSIGNALED(stat_loc)) 
        {
            printf("\nChild process (pid: %d) was terminated by signal %d\n", pid, WTERMSIG(stat_loc));
        }

        else 
        {
            printf("\nChild process (pid: %d) finished in unknown state\n", pid);
        }
    }

    return 0;
}