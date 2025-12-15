#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <libgen.h>

void print_permissions(mode_t mode) {
    char buf[11];

    buf[0] = S_ISDIR(mode) ? 'd' : S_ISREG(mode) ? '-' : '?';

    buf[1] = (mode & S_IRUSR) ? 'r' : '-';
    buf[2] = (mode & S_IWUSR) ? 'w' : '-';
    buf[3] = (mode & S_IXUSR) ? 'x' : '-';

    buf[4] = (mode & S_IRGRP) ? 'r' : '-';
    buf[5] = (mode & S_IWGRP) ? 'w' : '-';
    buf[6] = (mode & S_IXGRP) ? 'x' : '-';

    buf[7] = (mode & S_IROTH) ? 'r' : '-';
    buf[8] = (mode & S_IWOTH) ? 'w' : '-';
    buf[9] = (mode & S_IXOTH) ? 'x' : '-';

    buf[10] = '\0';

    printf("%s ", buf);
}

void print_file_info(const char *path) {
    struct stat st;

    if (stat(path, &st) != 0) {
        perror(path);
        return;
    }

    print_permissions(st.st_mode);

    printf("%3ld ", st.st_nlink);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);

    printf("%-10s %-10s ",
           pw ? pw->pw_name : "unknown",
           gr ? gr->gr_name : "unknown");

    if (S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))
        printf("%10ld ", st.st_size);
    else
        printf("%10s ", "");

    char timebuf[20];
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));
    printf("%s ", timebuf);

    char path_copy[1024];
    strncpy(path_copy, path, sizeof(path_copy));
    path_copy[sizeof(path_copy)-1] = '\0';

    printf("%s\n", basename(path_copy));
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        print_file_info(".");
        return 0;
    }

    for (int i = 1; i < argc; i++)
        print_file_info(argv[i]);

    return 0;
}
