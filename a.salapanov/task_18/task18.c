#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <libgen.h>
#include <unistd.h>
#include <limits.h>

static void print_file_info(const char *path) {
    struct stat st;

    /* lstat, чтобы для симлинков печатать сам линк, как делает ls -ld */
    if (lstat(path, &st) == -1) {
        perror(path);
        return;
    }

    /* Тип файла: d / - / ? */
    char type;
    if (S_ISDIR(st.st_mode))
        type = 'd';
    else if (S_ISREG(st.st_mode))
        type = '-';
    else
        type = '?';

    /* Права доступа rwx для user/group/other */
    char perms[10];
    perms[0] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    perms[1] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    perms[2] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    perms[3] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    perms[4] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    perms[5] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    perms[6] = (st.st_mode & S_IROTH) ? 'r' : '-';
    perms[7] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    perms[8] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    perms[9] = '\0';

    /* Владелец и группа */
    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);

    const char *user  = pw ? pw->pw_name  : "UNKNOWN";
    const char *group = gr ? gr->gr_name : "UNKNOWN";

    /* Время последней модификации */
    char timebuf[64];
    struct tm *mt = localtime(&st.st_mtime);
    if (mt != NULL) {
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", mt);
    } else {
        snprintf(timebuf, sizeof(timebuf), "????????????");
    }

    /* Получаем только имя файла из пути */
    char path_copy[PATH_MAX];
    strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';
    char *fname = basename(path_copy);

    /* Печать в «табличном» формате */
    printf("%c%s ", type, perms);                             // тип + права
    printf("%3lu ", (unsigned long)st.st_nlink);              // число ссылок
    printf("%-8s %-8s ", user, group);                        // владелец, группа

    if (S_ISREG(st.st_mode)) {
        printf("%8lld ", (long long)st.st_size);              // размер для обычных файлов
    } else {
        printf("%8s ", "");                                   // пустое поле для остальных
    }

    printf("%s ", timebuf);                                   // дата/время
    printf("%s\n", fname);                                    // имя файла
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <файл> [файл...]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        print_file_info(argv[i]);
    }

    return 0;
}
