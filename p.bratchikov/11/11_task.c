#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

extern char **environ;

void print_help() {
    printf("Custom execvpe Demonstration Program\n");
    printf("This program replaces the environment of a child process using a custom execvpe implementation.\n");
    printf("It then executes the 'date' command with the provided environment variables.\n\n");

    printf("Usage:\n");
    printf("  program VAR1=VALUE VAR2=VALUE ...\n");
    printf("  program -h\n");
    printf("  program --help\n\n");

    printf("Arguments:\n");
    printf("  VAR=VALUE     Environment variables to pass to the child process.\n");
    printf("  -h, --help    Show this help message.\n");
}

int execvpe(char *filename, char *arg[], char *envp[]) {
    char **ptr;

    if (envp == NULL) {
        fprintf(stderr, "Error: envp is NULL\n");
        return -1;
    }

    environ = envp;

    printf("New environ:\n");
    for (ptr = environ; *ptr != NULL; ptr++) {
        printf("%s\n", *ptr);
    }

    execvp(filename, arg);

    fprintf(stderr, "Error: execvp failed: %s\n", strerror(errno));
    return -1;
}

int main(int argc, char *argv[]) {

    if (argc == 2 && 
       ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0))) {
        print_help();
        return EXIT_SUCCESS;
    }

    if (argc < 2) {
        fprintf(stderr, "Error: at least one argument required.\n");
        fprintf(stderr, "Usage: program VAR=VALUE OR program -h\n");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid == -1) {
        fprintf(stderr, "Error: fork() failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        char *args[] = {"date", NULL};

        char **envp = argv + 1;
        int i;
        for (i = 1; i < argc; i++) {
            if (strchr(argv[i], '=') == NULL) {
                fprintf(stderr, "Error: invalid environment variable format: %s\n", argv[i]);
                return EXIT_FAILURE;
            }
        }

        if (execvpe("date", args, envp) == -1) {
            return EXIT_FAILURE;
        }

        return EXIT_FAILURE;
    }

    else {
        if (wait(NULL) == -1) {
            fprintf(stderr, "Error: wait() failed: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        printf("\nChild process (pid: %d) finished\n", pid);
    }

    return EXIT_SUCCESS;
}
