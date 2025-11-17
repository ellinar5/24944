#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static volatile sig_atomic_t count = 0;

void sigint_handler(int signo) {
    write(STDOUT_FILENO, "\a", 1);
    count++;
}

void sigquit_handler(int signo) {
    char buf[64];
    int len = snprintf(buf, sizeof(buf),
                       "\nПолучено сигналов: %d\n", count);
    write(STDOUT_FILENO, buf, len);
    _exit(0);
}

int main(void) {
    struct sigaction sa_int, sa_quit;

    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;  
    sigaction(SIGINT, &sa_int, NULL);

    sa_quit.sa_handler = sigquit_handler;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;          
    sigaction(SIGQUIT, &sa_quit, NULL);

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) <= 0) {
            break;
        }
    }

    return 0;
}
