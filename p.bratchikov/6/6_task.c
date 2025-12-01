#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

volatile bool timer_expired = false;

void handle_timer(int signal_num) { (void)signal_num; timer_expired = true; }

void print_help(const char *prog) {
    printf("This program reads up to 1024 bytes from a file and allows viewing specific lines.\n");
    printf("If no input is given within 5 seconds, the entire buffer is printed.\n\n");
    printf("Usage:\n");
    printf("  %s <filename>\n", prog);
    printf("  %s --help | -h\n", prog);
}

int main(int argc, char *argv[]) {


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


    char buffer[1024];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
    if (bytes_read < 0) {
        fprintf(stderr, "Error: failed to read from file.\n");
        close(fd);
        exit(EXIT_FAILURE);
    }
    if (bytes_read == 0) {
        printf("File is empty.\n");
        close(fd);
        return EXIT_SUCCESS;
    }


    int total_lines = 0;
    for (ssize_t i = 0; i < bytes_read; i++) {
        if (buffer[i] == '\n') total_lines++;
    }
    if (buffer[bytes_read-1] != '\n') total_lines++;


    int *line_offsets = malloc((total_lines + 1) * sizeof(int));
    if (!line_offsets) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    line_offsets[0] = 0;
    int idx = 1;
    for (ssize_t i = 0; i < bytes_read; i++) {
        if (buffer[i] == '\n') line_offsets[idx++] = i + 1;
    }
    line_offsets[total_lines] = bytes_read;


    printf("== String number to position mapping ==\n");
    for (int i = 0; i < total_lines; i++) {
        if (lseek(fd, line_offsets[i], SEEK_SET) == (off_t)-1) {
            fprintf(stderr, "Error: lseek failed.\n");
            free(line_offsets);
            close(fd);
            exit(EXIT_FAILURE);
        }

        int line_len = line_offsets[i+1] - line_offsets[i];
        char *line_buf = malloc(line_len + 1);
        if (!line_buf) {
            fprintf(stderr, "Error: memory allocation failed.\n");
            free(line_offsets);
            close(fd);
            exit(EXIT_FAILURE);
        }

        ssize_t r = read(fd, line_buf, line_len);
        if (r != line_len) {
            fprintf(stderr, "Error: failed to read full line.\n");
            free(line_buf);
            free(line_offsets);
            close(fd);
            exit(EXIT_FAILURE);
        }

        line_buf[line_len] = '\0';
        free(line_buf);

        printf("%d\t%d\t%d\n", i+1, line_offsets[i], line_len);
    }


    signal(SIGALRM, handle_timer);

    bool timer_active = false;


    while(1) {
        printf("Enter line number (1-%d, 0 to exit): ", total_lines);
        fflush(stdout);

        if (!timer_active) {
            timer_expired = false;
            alarm(5);
            timer_active = true;
        }

        int line_number;
        int res = scanf("%d", &line_number);
        alarm(0);

        if (timer_expired) {
            printf("\nFull file content:\n");
            write(STDOUT_FILENO, buffer, bytes_read);
            printf("\n");
            break;
        }

        if (res != 1) {
            fprintf(stderr, "Error: invalid input.\n");
            int c; while ((c=getchar()) != '\n' && c!=EOF);
            continue;
        }

        if (line_number == 0) break;
        if (line_number < 1 || line_number > total_lines) {
            fprintf(stderr, "Error: line number out of range.\n");
            continue;
        }

        int start = line_offsets[line_number-1];
        int len = line_offsets[line_number] - start;

        if (lseek(fd, start, SEEK_SET) == (off_t)-1) {
            fprintf(stderr, "Error: lseek failed.\n");
            continue;
        }

        char *line_buf = malloc(len + 1);
        if (!line_buf) {
            fprintf(stderr, "Error: memory allocation failed.\n");
            continue;
        }

        ssize_t r = read(fd, line_buf, len);
        if (r != len) {
            fprintf(stderr, "Error: failed to read line.\n");
            free(line_buf);
            continue;
        }

        line_buf[len] = '\0';
        printf("%s\n", line_buf);
        free(line_buf);
    }

    free(line_offsets);
    close(fd);
    return EXIT_SUCCESS;
}