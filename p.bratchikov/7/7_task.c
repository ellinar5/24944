#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>

volatile bool timeout = false;

void timeout_handler(int sig) { timeout = true; }

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int st = open(argv[1], O_RDONLY);
    if (st == -1) {
        perror("Failed to open file");
        return 1;
    }

    struct stat file_info;
    if (fstat(st, &file_info) == -1) {
        perror("Failed to get file info");
        close(st);
        return 1;
    }

    int file_size = file_info.st_size;
    if (file_size == 0) {
        printf("File is empty\n");
        close(st);
        return 0;
    }

    char *text = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, st, 0);
    if (text == MAP_FAILED) {
        perror("mmap failed");
        close(st);
        return 1;
    }

    int count_str = 0;
    for (int i = 0; i < file_size; i++) {
        if (text[i] == '\n')
            count_str++;
    }
    if (text[file_size - 1] != '\n')
        count_str++;

    int *pos_str = malloc((count_str + 1) * sizeof(int));
    if (!pos_str) {
        perror("malloc failed");
        munmap(text, file_size);
        close(st);
        return 1;
    }

    pos_str[0] = 0;
    int idx = 1;

    for (int i = 0; i < file_size; i++) {
        if (text[i] == '\n') {
            pos_str[idx] = i + 1;
            idx++;
        }
    }

    pos_str[count_str] = file_size;

    printf("== Таблица соответствия строки и номера ==\n");
    for (int j = 0; j < count_str; j++) {
        printf("%d\t%d\t%d\n", j + 1, pos_str[j], pos_str[j + 1] - pos_str[j]);
    }

    signal(SIGALRM, timeout_handler);
    int flag = 0;

    while (1) {
        printf("Введите номер строки (1-%d): ", count_str);
        fflush(stdout);

        if (flag == 0) {
            timeout = false;
            alarm(5);
            flag = 1;
        }

        int n;
        int result = scanf("%d", &n);

        alarm(0);

        if (timeout) {
            printf("\nВывод всего файла:\n");
            write(1, text, file_size);
            printf("\n");
            break;
        }

        if (result != 1) {
            printf("Invalid index\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        if (n == 0)
            break;

        if (n < 1 || n > count_str) {
            printf("Invalid index\n");
            continue;
        }

        for (int i = pos_str[n - 1]; i < pos_str[n]; i++)
            putchar(text[i]);
        putchar('\n');
    }

    free(pos_str);
    munmap(text, file_size);
    close(st);

    return 0;
}