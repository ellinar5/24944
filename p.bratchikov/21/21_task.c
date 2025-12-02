#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int count = 0;

void handleSIGINT(int sig) {
    printf("\a");
    fflush(NULL);
    count++;
}

void handleSIGQUIT(int sig) {
    printf("\nThe signal sounded %d times.\n", count);
    exit(0);
}

int main() {
    struct sigaction sa_int = {0};
    sa_int.sa_handler = handleSIGINT;
    sigaction(SIGINT, &sa_int, NULL);

    struct sigaction sa_quit = {0};
    sa_quit.sa_handler = handleSIGQUIT;
    sigaction(SIGQUIT, &sa_quit, NULL);

    while (1) pause();
}
