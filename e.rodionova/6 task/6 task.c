#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_LINES 1024
#define MAX_LINE_LEN 1024

int fd_global = -1; // Глобально для сигнала
off_t offsets[MAX_LINES];
int lengths[MAX_LINES];
int line_count = 0;
int first_input = 1; // флаг первого ввода

// Обработчик сигнала SIGALRM
void timeout_handler(int signum) {
    if (fd_global == -1) exit(1);

    printf("\n"); // перенос строки перед выводом файла
    lseek(fd_global, 0, SEEK_SET);
    char buf[MAX_LINE_LEN];
    ssize_t n;
    while ((n = read(fd_global, buf, sizeof(buf))) > 0) {
        write(STDOUT_FILENO, buf, n);
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
    fd_global = fd;

    // Построение таблицы смещений и длин строк
    off_t pos = 0, line_start = 0;
    char ch;
    while (read(fd, &ch, 1) == 1) {
        pos++;
        if (ch == '\n') {
            offsets[line_count] = line_start;
            lengths[line_count] = pos - line_start;
            line_start = pos;
            line_count++;
            if (line_count >= MAX_LINES) break;
        }
    }
    if (pos > line_start && line_count < MAX_LINES) {
        offsets[line_count] = line_start;
        lengths[line_count] = pos - line_start;
        line_count++;
    }

    // Установка обработчика сигнала
    signal(SIGALRM, timeout_handler);

    int n;
    char linebuf[MAX_LINE_LEN];

    while (1) {
        printf("Enter line number (0 to exit): ");
        fflush(stdout);

        // таймер только для первого ввода
        if (first_input) {
            alarm(5); // 5 секунд на ввод
        }

        if (scanf("%d", &n) != 1) break;

        if (first_input) alarm(0); // отключаем таймер после первого ввода
        first_input = 0;           // сбрасываем флаг

        if (n == 0) break;

        if (n < 1 || n > line_count) {
            printf("Invalid line number\n");
            continue;
        }

        off_t offset = offsets[n - 1];
        int len = lengths[n - 1];

        lseek(fd, offset, SEEK_SET);
        int bytes_read = read(fd, linebuf, len);
        if (bytes_read <= 0) {
            perror("read");
            continue;
        }

        linebuf[bytes_read] = '\0';
        printf("%s", linebuf);
        if (linebuf[bytes_read - 1] != '\n') printf("\n");
    }

    close(fd);
    return 0;
}