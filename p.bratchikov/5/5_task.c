#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

void print_help(const char *prog) {
    printf("This program reads up to 1024 bytes of a text file and allows you to view any line by its number.\n\n");
    printf("Usage:\n");
    printf("  %s <filename>\n", prog);
    printf("  %s --help\n", prog);
    printf("  %s -h\n", prog);
}

int main(int argc, char *argv[])
{
    if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) 
    {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }


    if (argc != 2) {
        fprintf(stderr, "Error: Expected exactly 1 argument.\n");
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Error: Failed to open file '%s'.\n", argv[1]);
        exit(EXIT_FAILURE);
    }


    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer));

    if (bytes_read < 0) {
        fprintf(stderr, "Error: Failed to read from file.\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (bytes_read == 0) {
        printf("File is empty.\n");
        close(fd);
        return EXIT_SUCCESS;
    }


    int line_count = 0;
    for (ssize_t i = 0; i < bytes_read; i++) {
        if (buffer[i] == '\n')
            line_count++;
    }
    if (buffer[bytes_read - 1] != '\n')
        line_count++;


    int *line_positions = malloc((line_count + 1) * sizeof(int));
    if (!line_positions) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    line_positions[0] = 0;
    int idx = 1;

    for (ssize_t i = 0; i < bytes_read; i++) {
        if (buffer[i] == '\n') {
            line_positions[idx++] = i + 1;
        }
    }

    line_positions[line_count] = bytes_read;

    printf("== Line number to position mapping ==\n");
    for (int i = 0; i < line_count; i++) {

        off_t seek_res = lseek(fd, line_positions[i], SEEK_SET);
        if (seek_res == (off_t)-1) {
            fprintf(stderr, "Error: lseek failed.\n");
            free(line_positions);
            close(fd);
            exit(EXIT_FAILURE);
        }

        int line_length = line_positions[i + 1] - line_positions[i];
        char *line_buf = malloc(line_length + 1);
        if (!line_buf) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            free(line_positions);
            close(fd);
            exit(EXIT_FAILURE);
        }

        ssize_t r = read(fd, line_buf, line_length);
        if (r != line_length) {
            fprintf(stderr, "Error: Failed to read full line.\n");
            free(line_buf);
            free(line_positions);
            close(fd);
            exit(EXIT_FAILURE);
        }

        line_buf[line_length] = '\0';

        free(line_buf);

        printf("%d\t%d\t%d\n", i + 1, line_positions[i], line_length);
    }


    while (1) {
        printf("Enter line number (1-%d, 0 to exit): ", line_count);

        int line_number;
        int res = scanf("%d", &line_number);

        if (res != 1) {
            fprintf(stderr, "Error: Invalid input.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        if (line_number == 0)
            break;

        if (line_number < 1 || line_number > line_count) {
            printf("Invalid line number.\n");
            continue;
        }

        int start = line_positions[line_number - 1];
        int length = line_positions[line_number] - start;

        if (lseek(fd, start, SEEK_SET) == (off_t)-1) {
            fprintf(stderr, "Error: lseek failed.\n");
            continue;
        }

        char *buf = malloc(length + 1);
        if (!buf) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            continue;
        }

        ssize_t r = read(fd, buf, length);
        if (r != length) {
            fprintf(stderr, "Error: Failed to read line.\n");
            free(buf);
            continue;
        }

        buf[length] = '\0';
        printf("Line %d: %s", line_number, buf);
        free(buf);
    }

    free(line_positions);
    close(fd);

    return EXIT_SUCCESS;
}