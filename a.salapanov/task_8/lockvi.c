// protected_editor.c
#define _XOPEN_SOURCE 700
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static void die(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <путь_к_файлу> [редактор]\n", argv[0]);
        fprintf(stderr, "Если редактор не указан, берётся $EDITOR, иначе nano/vi.\n");
        return 2;
    }
    const char *path = argv[1];

    // 1) Открываем файл для чтения и записи (обязательно O_RDWR для записи)
    int fd = open(path, O_RDWR | O_CLOEXEC);
    if (fd < 0) die("open(%s) failed: %s", path, strerror(errno));

    // 2) Ставим эксклюзивный блокировочный замок на весь файл (advisory/mandatory — зависит от среды)
    struct flock lk = {0};
    lk.l_type   = F_WRLCK;   // эксклюзивная блокировка на запись
    lk.l_whence = SEEK_SET;
    lk.l_start  = 0;
    lk.l_len    = 0;         // 0 = до конца файла (весь файл)
    if (fcntl(fd, F_SETLKW, &lk) == -1) {
        die("fcntl(F_SETLKW, WRLCK) failed: %s", strerror(errno));
    }

    // 3) Сообщим детали
    printf("[PID %ld] Захватил файл: %s (эксклюзивно). Нажмите Ctrl+C для теста блокировки другой программой.\n",
           (long)getpid(), path);

    // 4) Готовим команду редактора
    const char *editor = NULL;
    if (argc >= 3) {
        editor = argv[2];
    } else {
        editor = getenv("EDITOR");
        if (!editor || !*editor) editor = "nano";
    }

    // Безопасно экранируем имя файла для system(): минимально — в кавычки.
    // (На проде лучше execvp; тут оставим system() по условию.)
    char cmd[4096];
    snprintf(cmd, sizeof(cmd), "%s \"%s\"", editor, path);

    // 5) Запускаем редактор синхронно: пока редактор открыт — файл под замком.
    int rc = system(cmd);
    if (rc == -1) {
        die("system() failed: %s", strerror(errno));
    }

    // 6) Снимаем замок (по выходу редактора). Можно опустить — ядро снимет при close().
    lk.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &lk) == -1) {
        die("fcntl(F_SETLK, UNLCK) failed: %s", strerror(errno));
    }
    close(fd);

    printf("[PID %ld] Замок снят, выход.\n", (long)getpid());
    return WIFEXITED(rc) ? WEXITSTATUS(rc) : 1;
}
