#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

void print_help(void) {
    printf("Usage: program\n");
    printf("Generates 100 random numbers (0-99), sorts them, and prints 10 numbers per line.\n");
    printf("Sort is performed via 'sort -n' using pipes and fork.\n");
    printf("Options:\n");
    printf("  -h, --help   Display this help message\n");
}

int main(int argc, char* argv[]) {

    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_help();
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Error: unknown option '%s'\n", argv[1]);
            fprintf(stderr, "Use --help for usage.\n");
            return EXIT_FAILURE;
        }
    }

    int pipe_to_sort[2];     
    int pipe_from_sort[2];   

    if (pipe(pipe_to_sort) == -1 || pipe(pipe_from_sort) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        dup2(pipe_to_sort[0], STDIN_FILENO);
        dup2(pipe_from_sort[1], STDOUT_FILENO);

        close(pipe_to_sort[1]);
        close(pipe_from_sort[0]);

        execlp("sort", "sort", "-n", NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }

    close(pipe_to_sort[0]);
    close(pipe_from_sort[1]);

    FILE *sort_in = fdopen(pipe_to_sort[1], "w");
    if (!sort_in) {
        perror("fdopen");
        return EXIT_FAILURE;
    }

    srand(time(NULL));
    for (int i = 0; i < 100; i++) {
        fprintf(sort_in, "%d\n", rand() % 100);
    }

    fclose(sort_in); 

    char c;
    int count = 0;

    while (read(pipe_from_sort[0], &c, 1) == 1) {
        if (c == '\n') {
            printf(++count % 10 == 0 ? "\n" : " ");
            continue;
        }
        putchar(c);
    }

    close(pipe_from_sort[0]);

    wait(NULL);

    return EXIT_SUCCESS;
}