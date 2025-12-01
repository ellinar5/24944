#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

void print_help(const char *prog) {
    printf("This program demonstrates a pipe between parent and child.\n");
    printf("Parent sends text, child converts it to uppercase.\n");
    printf("Usage: %s [--help|-h]\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }

    int fildes[2];
    if (pipe(fildes) == -1) {
        fprintf(stderr, "pipe() failed\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        fprintf(stderr, "fork() failed\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        if (close(fildes[1]) == -1) {
            fprintf(stderr, "child close write end failed\n");
            exit(EXIT_FAILURE);
        }

        char c;
        ssize_t n;
        while ((n = read(fildes[0], &c, 1)) > 0) {
            if (write(STDOUT_FILENO, (char[]){(char)toupper(c)}, 1) != 1) {
                fprintf(stderr, "write failed in child\n");
                exit(EXIT_FAILURE);
            }
        }
        if (n == -1) {
            fprintf(stderr, "read failed in child\n");
            exit(EXIT_FAILURE);
        }

        if (close(fildes[0]) == -1) {
            fprintf(stderr, "child close read end failed\n");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    } else {
        if (close(fildes[0]) == -1) {
            fprintf(stderr, "parent close read end failed\n");
            exit(EXIT_FAILURE);
        }

        const char *text = "Hello, World!";
        size_t len = strlen(text);
        for (size_t i = 0; i < len; i++) {
            if (write(fildes[1], &text[i], 1) != 1) {
                fprintf(stderr, "write failed in parent\n");
                exit(EXIT_FAILURE);
            }
        }

        if (close(fildes[1]) == -1) {
            fprintf(stderr, "parent close write end failed\n");
            exit(EXIT_FAILURE);
        }

        int status;
        if (wait(&status) == -1) {
            fprintf(stderr, "wait() failed\n");
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}