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
    char permissions[10];

    permissions[0] = (S_ISDIR(mode)) ? 'd' : (S_ISREG(mode)) ? '-' : '?';
    permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-';
    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';
    permissions[7] = (mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';
    
    printf("%s ", permissions);
}

void print_file_info(const char *filename) {
    struct stat file_stat;
    if (stat(filename, &file_stat) == -1) {
        perror("stat");
        return;
    }

    print_permissions(file_stat.st_mode);

    printf("%ld ", file_stat.st_nlink);

    struct passwd *pw = getpwuid(file_stat.st_uid);
    printf("%-10s ", pw ? pw->pw_name : "Unknown");

    struct group *gr = getgrgid(file_stat.st_gid);
    printf("%-10s ", gr ? gr->gr_name : "Unknown");

    if (S_ISREG(file_stat.st_mode)) {
        printf("%-10ld ", file_stat.st_size);
    } else {
        printf("          ");
    }

    char time_str[20];
    strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&file_stat.st_mtime));
    printf("%s ", time_str);

    printf("%s\n", basename((char *)filename));
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> <file2> ...\n", argv[0]);
        return 1;
    }

    int i;
        for (i = 1; i < argc; i++) {
        print_file_info(argv[i]);
    }

    return 0;
}
