#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>

static volatile sig_atomic_t signal_count = 0;

void sigint_handler(int sig) {
    printf("\a");
    fflush(stdout);
    signal_count++;
}

void sigquit_handler(int sig) {
    printf("\nПрограмма завершена. Звуковой сигнал прозвучал %d раз(а).\n", signal_count);
    exit(0);
}

int main() {

    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigquit_handler);
    
    printf("Программа запущена. Используйте:\n");
    printf("  Ctrl+C - для звукового сигнала\n");
    printf("  Ctrl+\\ (или Ctrl+4) - для выхода\n");
    printf("Ожидание сигналов...\n");
    
    while(1) {
        pause();
    }
    
    return 0;
}