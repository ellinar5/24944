// two_proc.c
#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <sys/wait.h>   // wait, waitpid, WIFEXITED...
#include <unistd.h>     // fork, execlp
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    const char *path = "/etc/passwd";      // длинный файл по умолчанию (виден и на macOS)
    int do_wait = 0;

    // разбор аргументов: [файл] [--wait|-w]
    if (argc >= 2 && argv[1][0] != '-') {
        path = argv[1];
    }
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--wait") == 0 || strcmp(argv[i], "-w") == 0) {
            do_wait = 1;
        }
    }

    // печатаем до fork и сразу flush, чтобы буфер не дублировался у ребёнка
    printf("[parent] старт (pid=%ld), файл: %s\n", (long)getpid(), path);
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // child: заменить образ процесса на `cat`
        printf("[child ] запускаю: cat %s (pid=%ld)\n", path, (long)getpid());
        fflush(stdout);
        execlp("cat", "cat", path, (char*)0);
        // если вернулись — ошибка
        perror("[child ] execlp(cat)");
        _exit(127);
    }

    // parent
    printf("[parent] не последняя строка (child pid=%ld)\n", (long)pid);

    if (do_wait) {
        int status = 0;
        pid_t ret = waitpid(pid, &status, 0);   // ждём именно нашего ребёнка
        if (ret == -1) {
            perror("waitpid");
            return 1;
        }

        if (WIFEXITED(status)) {
            printf("[parent] child %ld завершился кодом %d\n",
                   (long)ret, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("[parent] child %ld убит сигналом %d\n",
                   (long)ret, WTERMSIG(status));
        } else {
            printf("[parent] child %ld завершился необычно (status=0x%x)\n",
                   (long)ret, status);
        }
    }

    // последняя строка: без --wait может выйти ДО cat, с --wait — строго ПОСЛЕ
    printf("[parent] последняя строка родителя\n");
    return 0;
}
