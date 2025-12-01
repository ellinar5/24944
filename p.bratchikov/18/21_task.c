#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

int beep_count = 0;

void sigint_handler(int sig) {
    signal(SIGINT, sigint_handler);
    beep_count++;
    printf("\r\aBEEP! \n");
    fflush(stdout);
}

void sigquit_handler(int sig) {
    printf("\r\e[2K");
    printf("\nProgram finished.\n");
    printf("Count of signals: %d\n", beep_count);
    exit(EXIT_SUCCESS);
}

void print_help(const char *prog) {
    printf("This program counts Ctrl+C signals and exits on Ctrl+\\\n");
    printf("Usage: %s [--help|-h]\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        fprintf(stderr, "Failed to set SIGINT handler\n");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGQUIT, sigquit_handler) == SIG_ERR) {
        fprintf(stderr, "Failed to set SIGQUIT handler\n");
        exit(EXIT_FAILURE);
    }

    printf("Program started. PID: %d\n", getpid());
    printf("Press Ctrl+C for sound signal\n");
    printf("Press Ctrl+\\ to show statistics and exit\n");
    printf("========================================\n");

    while (1) {
        pause();
    }

    return EXIT_SUCCESS;
}