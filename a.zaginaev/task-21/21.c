#include <stdio.h>
#include <signal.h>
#define   BELL_CHAR   '\07'

int interrupt_counter;

void handle_signal(int signal_type) {
    if (signal_type == SIGQUIT) {
        printf("The bell was triggered %d times\n", interrupt_counter);
        exit(0);
    }
    printf("%c\n", BELL_CHAR);
    interrupt_counter++;
    signal(signal_type, handle_signal);
}

int main() {
    signal(SIGINT, handle_signal);
    signal(SIGQUIT, handle_signal);

    for (;;) {
        pause();
    }

    return 0;
}
