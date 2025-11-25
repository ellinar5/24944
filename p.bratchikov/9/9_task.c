#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        printf("This program forks and executes: cat blur_faces.py\n");
        printf("Usage: %s [--help|-h]\n", argv[0]);
        return 0;
    }

    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "fork() error: %s\n", strerror(errno));
        return 1;
    }

    if (pid == 0) {
        execlp("cat", "cat", "blur_faces.py", NULL);

        fprintf(stderr, "execlp() error: %s\n", strerror(errno));
        return 1;
    }

    else {
        int status;
        if (wait(&status) == -1) {
            fprintf(stderr, "wait() error: %s\n", strerror(errno));
            return 1;
        }

        if (WIFEXITED(status)) 
        {
            printf("\nChild process (pid: %d) finished with exit code %d\n", pid, WEXITSTATUS(status));
        }

        else if (WIFSIGNALED(status)) 
        {
            printf("\nChild process (pid: %d) was terminated by signal %d\n", pid, WTERMSIG(status));
        }

        else 
            {
                printf("\nChild process (pid: %d) finished in unknown state\n", pid);
            }
    }

    return 0;
}