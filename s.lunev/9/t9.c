/*
 ============================================================================
 Name        : t9.c
 Author      : Sam Lunev
 Version     : .0
 Copyright   : All rights reserved
 Description : OSLrnCrsPT9
 ============================================================================
 */

 #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


void startVer(const char *file);
void stopVer(const char *file);


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    //printf("=== Start Version: Parent prints text before child execution ===\n");
    //startVer(argv[1]);
    
    printf("\n=== Stop Version: Parent waits for child, then prints text ===\n");
    stopVer(argv[1]);

    return 0;
}


void startVer(const char *file) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        execl("/bin/cat", "cat", file, NULL);
        perror("execl failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Child process PID: %d\n", pid);  
        printf("Parent: Child process has completed\n");
    }
}

void stopVer(const char *file) {
    int status = 0;
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        execl("/bin/cat", "cat", file, NULL);
        perror("execl failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Child process PID: %d\n", pid);
        
        pid_t result = waitpid(pid, &status, 0);
        if (result == -1) {
            perror("waitpid failed");
            exit(EXIT_FAILURE);
        }
        
        printf("Parent: Child process has completed\n");
    }
}
