#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>


extern char **environ; // доступ к переменным окружениям

int main(int argc, char *argv[])
{
    char options[ ] = "ispuU:cC:dvV:";  /* valid options */
    extern char *optarg; // указатель на аргумент опции
    extern int optopt;
    struct rlimit rlp; // структура для хранения ограничений системных ресурсов
    printf("argc equals %d\n", argc);
    int c;
    while ((c = getopt(argc, argv, options)) != -1){
        switch (c){
            // -i  Печатает реальные и эффективные идентификаторы пользователя и группы.
            case 'i':
                printf("Real User ID = %d\n", getuid());
                printf("Effective User ID = %d\n", geteuid());
                printf("Real Group ID = %d\n", getgid());
                printf("Effective Group ID = %d\n", getegid());
                printf("\n");
                break;
            // -s  Процесс становится лидером группы. Подсказка: смотри setpgid(2).
            case 's':
                // устанавливает процесс лидером группы (0 - текущий процесс, getpid() - PID процесса)
                if (setpgid(0, getpid()) == 0){ 
                    printf("New PGID: %d\n", getpgid(0)); 
                }
                else{ 
                    perror("Failed to set process group"); 
                }
                break;
            // -p  Печатает идентификаторы процесса, процесса-родителя и группы процессов.
            case 'p':
                printf("Process ID (pid) = %d\n", getpid());
                printf("Parent Process ID (ppid) = %d\n", getppid()); 
                printf("Process Group ID (pgid) = %d\n", getpgid(0));
                break;
            // -u  Печатает значение ulimit
            case 'u':
                {
                    struct rlimit rlim;
                    if (getrlimit(RLIMIT_NPROC, &rlim) == -1) { 
                        perror("Failed with getrlimit for FILE"); 
                    }
                    else
                    {
                        printf("File size limit (ulimit):\n");
                        printf(" Soft limit: %ld\n", (long)rlim.rlim_cur); // текущее
                        printf(" Hard limit: %ld\n", (long)rlim.rlim_max); // максимальное
                    }
                }
                break;

            // -Unew_ulimit  Изменяет значение ulimit. Подсказка: смотри atol(3C) на странице руководства strtol(3C)
            case 'U':
                if (getrlimit(RLIMIT_FSIZE, &rlp) == 0)
                {
                    rlp.rlim_cur = atol(optarg); // преобразует строку аргумента в long
                    if (setrlimit(RLIMIT_FSIZE, &rlp) == -1) { perror("Failed to change ulimit\n"); }
                    else { printf("Ulimit changed to: %ld\n", rlp.rlim_cur); }
                }
                else { perror("Failed to get current ulimit\n"); }
                break;
            // -c  Печатает размер в байтах core-файла, который может быть создан.
            case 'c':
                {
                    struct rlimit rlim;
                    // получаем SOFT and HARD ограничения на размеры core файлов
                    if (getrlimit(RLIMIT_CORE, &rlim) == 0)
                    {
                        // размер в байтах
                        printf("Soft limit: %lu\n", (unsigned long)rlim.rlim_cur); // ulimit -S -c
                        printf("Hard limit: %lu\n", (unsigned long)rlim.rlim_max); // ulimit -H -c
                    }
                    else { perror("Failed to get core file limit"); }
                }
                break;
            // -Csize  Изменяет размер core-файла
            case 'C':
                if (getrlimit(RLIMIT_CORE, &rlp) == 0)
                {
                    rlp.rlim_cur = atol(optarg);
                    if (setrlimit(RLIMIT_CORE, &rlp) == -1) { 
                        perror("Failed to change core file size\n"); 
                    }
                    else
                    {
                        printf("New Soft limit: %lu\n", rlp.rlim_cur);
                        printf("Hard limit: %lu\n", rlp.rlim_max);
                    }
                }
                else { 
                    perror("Failed to get current core file limit\n"); 
                }
                break;
            // -d  Печатает текущую рабочую директорию
            case 'd':
                {
                    char pwd[1024];
                    if (getcwd(pwd, sizeof(pwd)) != NULL) { 
                        printf("Current wd: %s\n", pwd); 
                    }
                    else { 
                        perror("Failed to get current directory\n"); 
                    }
                }
                break;
            // -v  Распечатывает переменные среды и их значения
            case 'v':
                printf("Environment variables:\n");
                for (char** env = environ; *env != NULL; env++) { 
                    printf("  %s\n", *env); 
                }
                break;
            // -Vname=value  Вносит новую переменную в среду или изменяет значение существующей переменной.
            case 'V':
                // добавляет или изменяет переменную окружения
                if (putenv(optarg) == 0) { 
                    printf("Environment variable set successfully: %s\n", optarg); 
                }
                else { 
                    perror("Failed to set environment variable\n"); 
                }
                break;
            // Иначе
            case '?':
                fprintf(stderr, "Unknown option: %c\n", optopt);
                break;
        }
    }
    return 0;
}