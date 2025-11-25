#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int beep_count = 0;

void sigint_handler(int sig){
    printf("\a beep\n"); 
    fflush(stdout); 
    beep_count++;
}

void sigquit_handler(int sig){
    printf("\nTotal beep signals: %d\n", beep_count);
    exit(0);
}

int main(){
    struct sigaction sa_int, sa_quit;
    // Ctrl+C
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask); 
	sa_int.sa_flags = 0;
    // Ctrl+/
    sa_quit.sa_handler = sigquit_handler;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;

    sigaction(SIGINT, &sa_int, NULL);
    sigaction(SIGQUIT, &sa_quit, NULL);
    
    printf("Count: %d\n", beep_count);
    while (1) { pause(); }
    return 0;
}