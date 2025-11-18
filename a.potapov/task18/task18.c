#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

void print_file_info(const char *filename)
{
    struct stat st;
    char data[11];
    char time_data[20];

    if (lstat(filename, &st) == -1){
        perror("lstat");
        return;
    }
    
    if(S_ISDIR(st.st_mode)){
        data[0] = 'd';
    }
    else if(S_ISREG(st.st_mode)){
        data[0] = '-';
    }
    else{
        data[0] = '?';
    }
    
    if((st.st_mode & S_IRUSR)){
        data[1] = 'r';
    }
    else{
        data[1] = '-';
    }

    if((st.st_mode & S_IWUSR)){
        data[2] = 'w';
    }
    else{
        data[2] = '-';
    }
 
    if((st.st_mode & S_IXUSR)){
        data[3] = 'x';
    }
    else{
        data[3] = '-';
    }

    if((st.st_mode & S_IRGRP)){
        data[4] = 'r';
    }
    else{
        data[4] = '-';
    }

    if((st.st_mode & S_IWGRP)){
        data[5] = 'w';
    }
    else{
        data[5] = '-';
    }

    if((st.st_mode & S_IXGRP)){
        data[6] = 'x';
    }
    else{
        data[6] = '-';
    }

    if(st.st_mode & S_IROTH){
        data[7] = 'r';
    }
    else{
        data[7] = '-';
    }

    if(st.st_mode & S_IWOTH){
        data[8] = 'w';
    }
    else{
        data[8] = '-';
    }

    if(st.st_mode & S_IXOTH){
        data[9] = 'x';
    }
    else{
        data[9] = '-';
    }
    data[10] = '\0';

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);

    struct tm *tm_info = localtime(&st.st_mtime);
    strftime(time_data, sizeof(time_data), "%b %d %H:%M", tm_info);

    const char *name = strrchr(filename, '/');
    if (name != NULL){
        name++;
    }
    else{
        name = filename;
    }
    printf("%s %lu %-10.10s %-10.10s ", data, st.st_nlink,
           pw ? pw->pw_name : "?",
           gr ? gr->gr_name : "?");

    printf("%8ld ", (long)st.st_size);
    printf("%s %s\n", time_data, name);
}

int main(int argc, char *argv[])
{
    if (argc < 2){
        print_file_info("Нет файла ил он указан некорректно");
    }
    else{
        for (int i = 1; i < argc; i++){
            print_file_info(argv[i]);
        }
    }
    return 0;
}