#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

void print_file_info(const char *path) {
    struct stat st;
    struct passwd *pw;
    struct group *gr;
    char permissions[11];
    char time_buf[20];
    char type_char;
    const char *filename;
    
    if (lstat(path, &st) == -1) {
        perror("lstat");
        return;
    }
    
    if (S_ISDIR(st.st_mode)) {
        type_char = 'd';
    } else if (S_ISREG(st.st_mode)) {
        type_char = '-';
    } else {
        type_char = '?';
    }
    
    permissions[0] = type_char;
    permissions[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    permissions[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    permissions[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    permissions[10] = '\0';
    

    pw = getpwuid(st.st_uid);
    gr = getgrgid(st.st_gid);

    struct tm *timeinfo = localtime(&st.st_mtime);
    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", timeinfo);
    
    filename = strrchr(path, '/');
    if (filename != NULL) {
        filename++;
    } else {
        filename = path;
    }
    
    printf("%-10s %2lu %-8s %-8s ", permissions, st.st_nlink,
           pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");
    
    if (S_ISREG(st.st_mode)) {
        printf("%8ld ", st.st_size);
    } else {
        printf("%8s ", "");
    }
    
    printf("%s %s\n", time_buf, filename);
}

int main(int argc, char *argv[]) {
    
    for (int i = 1; i < argc; i++) {
        print_file_info(argv[i]);
    }
    
    return 0;
}