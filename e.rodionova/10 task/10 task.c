#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

void print_help(const char *prog) {
    printf("This program forks and executes: cat blur_faces.py\n");
    printf("Usage: %s [--help|-h]\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }

    pid_t pid = fork();
    if (pid == -1) {
        fprintf(stderr, "fork() error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        execlp("cat", "cat", "blur_faces.py", NULL);
        fprintf(stderr, "execlp() error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int status;
    if (wait(&status) == -1) {
        fprintf(stderr, "wait() error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (WIFEXITED(status)) {
        printf("Child process (pid: %d) finished with exit code %d\n", pid, WEXITSTATUS(status));
    } 
    else if (WIFSIGNALED(status)) {
        printf("Child process (pid: %d) was terminated by signal %d\n", pid, WTERMSIG(status));
    } 
    else {
        printf("Child process (pid: %d) finished in unknown state\n", pid);
    }

    return EXIT_SUCCESS;
}