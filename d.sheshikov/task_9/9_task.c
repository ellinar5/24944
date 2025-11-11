#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int wstatus;
    pid_t pid = fork();

    if (pid == 0) {
        execlp("cat", "cat", "text.txt", NULL);
    }

    else if (pid > 0) {

        for (int i = 1; i <= 5; i++) {
            printf("Родитель: сообщение %d\n", i);}
        printf("\n");
        
        pid_t w = wait(&wstatus);
        
        printf("\n \n Строка выведена после заврешения подпроцесса с ID: %d\n", w);
    }

    return 0;
}