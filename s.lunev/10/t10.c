/*
 ============================================================================
 Name        : t10.c
 Author      : Sam Lunev
 Version     : .0
 Copyright   : All rights reserved
 Description : OSLrnCrsPT10
 ============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s command [arguments...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execvp(argv[1], &argv[1]);
        
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else {
        int status;
        pid_t result;
        
        result = waitpid(pid, &status, 0);
        
        if (result == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Command '%s' finished with exit code: %d\n", argv[1], exit_code);
        }
        else if (WIFSIGNALED(status)) {
            int signal_num = WTERMSIG(status);
            printf("Command '%s' was terminated by signal: %d\n", argv[1], signal_num);
        }
        else {
            printf("Command '%s' finished with unknown status\n", argv[1]);
        }
    }
    
    return 0;
}
