/*
 ============================================================================
 Name        : t21.c
 Author      : Sam Lunev
 Version     : .0
 Copyright   : All rights reserved
 Description : OSLrnCrsPT21
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

static int beep_count = 0;


void sigint_handler(int sig)
{
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        fprintf(stderr, "Error reinstalling SIGINT handler: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    beep_count++;
    printf("\r\aBEEP! \n");
    fflush(stdout);
}

void sigquit_handler(int sig)
{
    printf("\r\e[2K");
    printf("\nProgram finished.\n");
    printf("Count of signals: %d\n", beep_count);
    
    exit(EXIT_SUCCESS);
}


int main(void)
{
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        fprintf(stderr, "Error setting SIGINT handler: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    
    if (signal(SIGQUIT, sigquit_handler) == SIG_ERR) {
        fprintf(stderr, "Error setting SIGQUIT handler: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    printf("Program started. PID: %d\n", getpid());
    printf("Press Ctrl+C for sound signal\n");
    printf("Press Ctrl+\\ to show statistics and exit\n");
    printf("========================================\n");

    while (1) {
        if (pause() == -1) {
            if (errno != EINTR) {
                fprintf(stderr, "Error in pause(): %s\n", strerror(errno));
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}
