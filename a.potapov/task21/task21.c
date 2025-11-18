#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int beep_count = 0;

void sig_ctrl_c(int sig)
{
    signal(SIGINT, sig_ctrl_c);
    beep_count++;
    printf("\r\aBEEP! \n");
    fflush(stdout);
}

void sig_ctrl_q(int sig)
{
    printf("\r\e[2K");
    printf("\nCount of signals: %d\n", beep_count);
    exit(0);
}

int main()
{
    signal(SIGINT, sig_ctrl_c);
    signal(SIGQUIT, sig_ctrl_q);

    printf("Ctrl+C - звуковой сигнал\n");
    printf("Ctrl+\\ - вывод количества и зваршение\n");
    printf("****************************************\n");

    while (1){
        pause();
    }
    return 0;
}