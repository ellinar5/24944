#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

volatile bool timeout = false;

void timeout_handler(int sig) { (void)sig; timeout = true; }

void print_help(const char *prog) {
    printf("This program maps a file into memory and allows viewing specific lines.\n");
    printf("If no input is given within 5 seconds, the entire file is printed.\n\n");
    printf("Usage:\n");
    printf("  %s <filename>\n", prog);
    printf("  %s --help | -h\n", prog);
}

int main(int argc, char *argv[])
{
    if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }

    if (argc != 2) {
        fprintf(stderr, "Error: expected exactly 1 argument.\n");
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Error: failed to open file '%s'.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    struct stat file_info;
    if (fstat(fd, &file_info) == -1) {
        fprintf(stderr, "Error: failed to get file info.\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    ssize_t file_size = file_info.st_size;
    if (file_size == 0) {
        printf("File is empty.\n");
        close(fd);
        return EXIT_SUCCESS;
    }


    char *text = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (text == MAP_FAILED) {
        fprintf(stderr, "Error: mmap failed.\n");
        close(fd);
        exit(EXIT_FAILURE);
    }


    int count_str = 0;
    for (ssize_t i = 0; i < file_size; i++)
        if (text[i] == '\n') count_str++;
    if (text[file_size-1] != '\n') count_str++;


    int *pos_str = malloc((count_str + 1) * sizeof(int));
    if (!pos_str) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        munmap(text, file_size);
        close(fd);
        exit(EXIT_FAILURE);
    }

    pos_str[0] = 0;
    int idx = 1;
    for (ssize_t i = 0; i < file_size; i++) {
        if (text[i] == '\n') pos_str[idx++] = i + 1;
    }
    pos_str[count_str] = file_size;

    printf("== String number to position mapping ==\n");
    for (int i = 0; i < count_str; i++) {
        printf("%d\t%d\t%d\n", i+1, pos_str[i], pos_str[i+1]-pos_str[i]);
    }


    if (signal(SIGALRM, timeout_handler) == SIG_ERR) {
        fprintf(stderr, "Error: failed to set signal handler.\n");
        free(pos_str);
        munmap(text, file_size);
        close(fd);
        exit(EXIT_FAILURE);
    }

    bool timer_active = false;


    while(1) {
        printf("Enter line number (1-%d, 0 to exit): ", count_str);
        fflush(stdout);

        if (!timer_active) {
            timeout = false;
            alarm(5);
            timer_active = true;
        }

        int n;
        int res = scanf("%d", &n);
        alarm(0);

        if (timeout) {
            printf("\nFull file content:\n");
            ssize_t w = write(STDOUT_FILENO, text, file_size);
            if (w != file_size) {
                fprintf(stderr, "Error: failed to write full file.\n");
                free(pos_str);
                munmap(text, file_size);
                close(fd);
                exit(EXIT_FAILURE);
            }
            printf("\n");
            break;
        }

        if (res != 1) {
            fprintf(stderr, "Error: invalid input.\n");
            int c; while ((c=getchar()) != '\n' && c!=EOF);
            continue;
        }

        if (n == 0) break;
        if (n < 1 || n > count_str) {
            fprintf(stderr, "Error: line number out of range.\n");
            continue;
        }

        for (int i = pos_str[n-1]; i < pos_str[n]; i++)
            putchar(text[i]);
        putchar('\n');
    }

    free(pos_str);
    munmap(text, file_size);
    close(fd);

    return EXIT_SUCCESS;
}