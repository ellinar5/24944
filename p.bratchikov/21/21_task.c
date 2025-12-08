#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static volatile sig_atomic_t count = 0;
static volatile sig_atomic_t using_sigaction = 0;

void handle_sigint(int sig) {
    if (!using_sigaction) signal(SIGINT, handle_sigint);
    printf("\a");
    fflush(stdout);
    count++;
}

void handle_sigquit(int sig) {
    if (!using_sigaction) signal(SIGQUIT, handle_sigquit);
    printf("\nThe signal sounded %d times.\n", (int)count);
    exit(EXIT_SUCCESS);
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
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        if (signal(SIGINT, handle_sigint) == SIG_ERR) {
            fprintf(stderr, "Error: cannot set SIGINT handler\n");
            return EXIT_FAILURE;
        }
        using_sigaction = 0;
    } else {
        using_sigaction = 1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigquit;
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        if (signal(SIGQUIT, handle_sigquit) == SIG_ERR) {
            fprintf(stderr, "Error: cannot set SIGQUIT handler\n");
            return EXIT_FAILURE;
        }
        using_sigaction = 0;
    } else {
        using_sigaction = 1;
    }

    while (1);

    return EXIT_SUCCESS;
}