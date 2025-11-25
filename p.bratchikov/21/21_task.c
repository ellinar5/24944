#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

volatile sig_atomic_t beep_count = 0;

void sigint_handler(int sig) {
    beep_count++;
    printf("\r\aBEEP!\n");
    fflush(stdout);
}

void sigquit_handler(int sig) {
    printf("\r\e[2K");
    printf("\nProgram finished.\n");
    printf("Count of signals: %d\n", beep_count);
    exit(0);
}

void print_help(const char *prog_name) 
{
    printf("\nThis program counts SIGINT signals (Ctrl+C) and exits on SIGQUIT (Ctrl+\\).\n");
    printf("\nUsage: %s [--help|-h]\n", prog_name);
    printf("\nPress Ctrl+C for sound signal\n");
    printf("\nPress Ctrl+\\ to show statistics and exit\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) 
    {
        print_help(argv[0]);
        return 0;
    }

    if (signal(SIGINT, sigint_handler) == SIG_ERR) 
    {
        fprintf(stderr, "\nError setting SIGINT handler: %s\n", strerror(errno));
        return 1;
    }

    if (signal(SIGQUIT, sigquit_handler) == SIG_ERR) 
    {
        fprintf(stderr, "\nError setting SIGQUIT handler: %s\n", strerror(errno));
        return 1;
    }

    printf("\nProgram started. PID: %d\n", getpid());

    while (1) {
        pause();
    }

    return 0;
}
