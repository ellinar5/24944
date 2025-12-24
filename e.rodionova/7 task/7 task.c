#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAX_LINES 1024

char *file_data = NULL;
size_t file_size = 0;
off_t offsets[MAX_LINES];
int lengths[MAX_LINES];
int line_count = 0;

void timeout_handler(int signum) {
    printf("\n");
    for (int i = 0; i < line_count; i++) {
        fwrite(file_data + offsets[i], 1, lengths[i], stdout);
        if (file_data[offsets[i] + lengths[i] - 1] != '\n') printf("\n");
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat failed");
        close(fd);
        return 1;
    }
    file_size = st.st_size;

    file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_data == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        return 1;
    }
    close(fd);

    off_t line_start = 0;
    for (size_t pos = 0; pos < file_size; pos++) {
        if (file_data[pos] == '\n') {
            offsets[line_count] = line_start;
            lengths[line_count] = pos - line_start + 1;
            line_start = pos + 1;
            line_count++;
            if (line_count >= MAX_LINES) break;
        }
    }

    if (line_start < file_size && line_count < MAX_LINES) {
        offsets[line_count] = line_start;
        lengths[line_count] = file_size - line_start;
        line_count++;
    }

    signal(SIGALRM, timeout_handler);

    int n;
    char dummy; 

    while (1) {
        printf("Enter line number (0 to exit): ");
        fflush(stdout);

        alarm(5);
        if (scanf("%d", &n) != 1) break;
        while ((dummy = getchar()) != '\n' && dummy != EOF);
        alarm(0);

        if (n == 0) break;
        if (n < 1 || n > line_count) {
            printf("Invalid line number\n");
            continue;
        }

        fwrite(file_data + offsets[n - 1], 1, lengths[n - 1], stdout);
        if (file_data[offsets[n - 1] + lengths[n - 1] - 1] != '\n') printf("\n");
    }

    munmap(file_data, file_size);
    return 0;
}