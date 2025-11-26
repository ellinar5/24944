/* v2_wait.c
 * Дочерний: exec cat /etc/services
 * Родитель: печатает несколько строк, затем ЖДЁТ (waitpid) и печатает финальную строку
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0); /* без буферизации для наглядности */

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        execlp("cat", "cat", "/etc/services", (char *)NULL);
        perror("execlp(cat)");
        _exit(127);
    }

    /* Родитель печатает несколько строк параллельно с cat */
    printf("[parent] Начинаю печатать строки...\n");
    for (int i = 1; i <= 5; ++i) {
        printf("[parent] строка %d\n", i);
    }

    /* КЛЮЧ: ждём завершения дочернего процесса */
    int status = 0;
    pid_t w = waitpid(pid, &status, 0);
    if (w == -1) {
        perror("waitpid");
        return 1;
    }

    /* Финальная строка — теперь строго ПОСЛЕ завершения cat */
    if (WIFEXITED(status)) {
        printf("[parent] Дочерний завершился: exit=%d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("[parent] Дочерний убит сигналом %d\n", WTERMSIG(status));
    }
    printf("[parent] Финальная строка после дочернего ✅\n");
    return 0;
}
