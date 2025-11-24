#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <libgen.h>


void print_file_info(const char *path) {
    struct stat st;

    if (lstat(path, &st) == -1) {
        perror(path);
        return;
    }

 
    char type;
    if      (S_ISDIR(st.st_mode))  type = 'd';
    else if (S_ISREG(st.st_mode))  type = '-';
    else                           type = '?';


    char perm[10];
    perm[0] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    perm[1] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    perm[2] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    perm[3] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    perm[4] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    perm[5] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    perm[6] = (st.st_mode & S_IROTH) ? 'r' : '-';
    perm[7] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    perm[8] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    perm[9] = '\0';


    int links = st.st_nlink;


    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    const char *user = pw ? pw->pw_name : "?";
    const char *group = gr ? gr->gr_name : "?";


    long size = S_ISREG(st.st_mode) ? st.st_size : -1;


    char date[32];
    struct tm *tm_info = localtime(&st.st_mtime);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M", tm_info);


    char path_copy[256];
    strncpy(path_copy, path, sizeof(path_copy));
    path_copy[sizeof(path_copy) - 1] = '\0';
    char *name = basename(path_copy);

    printf("%c%s %3d %-10s %-10s ", type, perm, links, user, group);

    if (size != -1)
        printf("%8ld ", size);
    else
        printf("         ");  

    printf("%s %s\n", date, name);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s FILE...\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        print_file_info(argv[i]);
    }

    return 0;
}
