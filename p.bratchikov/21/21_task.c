#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

static volatile sig_atomic_t beep_count = 0;


void sigint_handler(int sig)
{
    (void)sig;
    beep_count++;


    if (write(STDOUT_FILENO, "\a\nBEEP!\n",8) == -1) {
    }
}


void sigquit_handler(int sig)
{
    (void)sig;

    char buf[128];
    int len = snprintf(buf, sizeof(buf), "\nProgram finished.\nCount of signals: %d\n", beep_count);

    if (len > 0)
        write(STDOUT_FILENO, buf, (size_t)len);

    _exit(EXIT_SUCCESS);
}

void print_help(const char *prog)
{
    printf("Usage: %s\n", prog);
    printf("Ctrl+C  - sound signal (beep)\n");
    printf("Ctrl+\\  - print statistics and exit\n");
}

int main(int argc, char *argv[])
{

    if (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }

    if (argc != 1) {
        fprintf(stderr, "Invalid arguments. Use --help or -h\n");
        return EXIT_FAILURE;
    }

    struct sigaction sa_int, sa_quit;


    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;

    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction(SIGINT)");
        return EXIT_FAILURE;
    }


    sa_quit.sa_handler = sigquit_handler;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;

    if (sigaction(SIGQUIT, &sa_quit, NULL) == -1) {
        perror("sigaction(SIGQUIT)");
        return EXIT_FAILURE;
    }

    while (1)
        pause();
}