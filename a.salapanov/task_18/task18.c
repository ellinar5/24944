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

static void build_perm_string(mode_t mode, char *out) {
    const char chars[] = "rwxrwxrwx";
    for (int i = 0; i < 9; i++) {
        out[i] = (mode & (1 << (8 - i))) ? chars[i] : '-';
    }
    out[9] = '\0';
}

static char file_type_char(mode_t mode) {
    if (S_ISDIR(mode))  return 'd';
    if (S_ISREG(mode))  return '-';
    return '?';
}

static void print_file_info(const char *path) {
    struct stat st;

    if (lstat(path, &st) == -1) {
        perror(path);
        return;
    }

    char perms[10];
    build_perm_string(st.st_mode, perms);
    char type = file_type_char(st.st_mode);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);

    const char *user  = pw ? pw->pw_name  : "UNKNOWN";
    const char *group = gr ? gr->gr_name  : "UNKNOWN";

    char timebuf[64];
    struct tm *mt = localtime(&st.st_mtime);
    if (mt) {
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", mt);
    } else {
        snprintf(timebuf, sizeof(timebuf), "????????????");
    }

    char path_copy[PATH_MAX];
    strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';
    char *fname = basename(path_copy);

    printf("%c%s ", type, perms);                      
    printf("%3lu ", (unsigned long)st.st_nlink);      
    printf("%-8s %-8s ", user, group);                

    
    printf("%8lld ", (long long)st.st_size);       
    

    printf("%s ", timebuf);                             
    printf("%s\n", fname);                            
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
