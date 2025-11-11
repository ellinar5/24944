#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

void print_file_info(const char *filename)
{
    struct stat st;
    
    if (lstat(filename, &st) == -1) { perror("lstat"); return; }
    
    // тип файла
    char type;
    if (S_ISDIR(st.st_mode)) { type = 'd'; }
    else if (S_ISREG(st.st_mode)) { type = '-'; }
    else { type = '?'; }
    
    // права доступа
    char permissions[10];
    permissions[0] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    permissions[1] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    permissions[2] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    permissions[3] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    permissions[4] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    permissions[5] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    permissions[6] = (st.st_mode & S_IROTH) ? 'r' : '-';
    permissions[7] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    permissions[8] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    permissions[9] = '\0';
    
    // имя владельца и группы
    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    
    char owner[32] = "?";
    char group[32] = "?";
    if (pw != NULL) strncpy(owner, pw->pw_name, sizeof(owner) - 1);
    if (gr != NULL) strncpy(group, gr->gr_name, sizeof(group) - 1);
    
    // время модификации
    char time_buf[32];
    struct tm *tm_info = localtime(&st.st_mtime);
    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", tm_info);
    
    // имя файла (без пути)
    char *basename = strrchr(filename, '/');
    if (basename != NULL) basename++;
    else basename = (char *)filename;

    // вывод информации
    if (S_ISREG(st.st_mode)) // обычный файл
    {
        
        printf("%c%s %2ld %-8s %-8s %8ld %s %s\n", type, permissions, st.st_nlink, owner, group, st.st_size, time_buf, basename);
    }
    else
    {
        printf("%c%s %2ld %-8s %-8s %8ld %s %s\n", type, permissions, st.st_nlink, owner, group, st.st_size, time_buf, basename);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) { fprintf(stderr, "Not enough arguments\n"  ); return 1; }
    
    for (int i = 1; i < argc; i++) { print_file_info(argv[i]); }
    
    return 0;
}