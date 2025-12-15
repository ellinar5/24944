#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t count = 0;

void handle_sigint(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "\a", 1);
    count++;
}

void handle_sigquit(int sig) {
    (void)sig;

    char buf[128];
    int len = snprintf(buf, sizeof(buf),
                       "\nThe signal sounded %d times.\n",
                       (int)count);

    write(STDOUT_FILENO, buf, len);

    _exit(0);  
}

int main(int argc, char **argv) {

    if (argc == 2) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("This program installs handlers for SIGINT and SIGQUIT.\n");
            printf("Press Ctrl+C to produce a beep and increment the counter.\n");
            printf("Press Ctrl+\\ to print the number of beeps and exit.\n");
            printf("Usage: %s [--help | -h]\n", argv[0]);
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Error: unknown option '%s'\n", argv[1]);
            return EXIT_FAILURE;
        }
    } else if (argc > 2) {
        fprintf(stderr, "Error: too many arguments\n");
        return EXIT_FAILURE;
    }

    struct sigaction sa;


    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;          
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT");
        return EXIT_FAILURE;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigquit;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        perror("sigaction SIGQUIT");
        return EXIT_FAILURE;
    }

    for (;;) {
        pause();
    }

    return EXIT_SUCCESS;
}
