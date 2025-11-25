/* v1_no_wait.c
 * Дочерний: exec cat /etc/services
 * Родитель: печатает несколько строк параллельно (без ожидания)
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(void) {
    /* Чтобы увидеть «перемешивание» вывода, отключим буферизацию stdout */
    setvbuf(stdout, NULL, _IONBF, 0);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        /* Дочерний процесс: заменить себя на cat */
        execlp("cat", "cat", "/etc/services", (char *)NULL);
        /* Если сюда дошли — exec не удался */
        perror("execlp(cat)");
        _exit(127);
    }

    /* Родительский процесс: печатает свой текст, пока cat работает */
    printf("[parent] Начинаю печатать строки...\n");
    for (int i = 1; i <= 5; ++i) {
        printf("[parent] строка %d\n", i);
    }
    printf("[parent] Готово (без ожидания завершения дочернего)\n");
    return 0;
}
