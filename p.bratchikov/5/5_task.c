#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int file_descriptor = open(argv[1], O_RDONLY);
    if (file_descriptor == -1) {
        perror("Failed to open file");
        return 1;
    }

    char buffer[1024];
    int bytes_read = read(file_descriptor, buffer, sizeof(buffer));
    if (bytes_read == -1) {
        perror("Failed to read file");
        close(file_descriptor);
        return 1;
    }

    int line_count = 0;
    int i;
    for (i = 0; i < bytes_read; i++) {
        if (buffer[i] == '\n') {
            line_count++;
        }
    }

    if (bytes_read > 0 && buffer[bytes_read-1] != '\n') {
        line_count++;
    }

    if (line_count == 0) {
        printf("File is empty\n");
        close(file_descriptor);
        return 0;
    }

    int *line_positions = malloc((line_count + 1) * sizeof(int));
    if (line_positions == NULL) {
        perror("Memory allocation failed");
        close(file_descriptor);
        return 1;
    }
    
    line_positions[0] = 0;
    int index = 1;

    for (i = 0; i < bytes_read; i++) {
        if (buffer[i] == '\n') {
            line_positions[index] = i + 1;
            index++;
        }
    }

    line_positions[line_count] = bytes_read;

    printf("== Line number to position mapping ==\n");
    int j;
    for (j = 0; j < line_count; j++) {
        lseek(file_descriptor, line_positions[j], SEEK_SET);

        int line_length = line_positions[j+1] - line_positions[j];
        char *line_content = malloc(line_length + 1);
        if (line_content) {
            read(file_descriptor, line_content, line_length);
            line_content[line_length] = '\0';
            free(line_content);
        }

        printf("%d\t%d\t%d\n", j+1, line_positions[j], line_length);
    }

    while (1) {
        printf("Enter line number (1-%d, 0 to exit): ", line_count);
        int line_number;
        int scan_result = scanf("%d", &line_number);

        if (scan_result != 1) {
            printf("Error: Invalid input\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        if (line_number == 0) break;

        if (line_number < 1 || line_number > line_count) {
            printf("Invalid line number\n");
            continue;
        }

        lseek(file_descriptor, line_positions[line_number-1], SEEK_SET);

        int line_length = line_positions[line_number] - line_positions[line_number-1];
        char *line_content = malloc(line_length + 1);
        if (line_content == NULL) {
            perror("Memory allocation failed");
            continue;
        }

        read(file_descriptor, line_content, line_length);
        line_content[line_length] = '\0';

        printf("Line %d: %s", line_number, line_content);
        free(line_content);
    }

    free(line_positions);
    close(file_descriptor);

    return 0;
}