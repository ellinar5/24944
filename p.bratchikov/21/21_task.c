#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define LINE_LENGTH 40

static struct termios orig_termios;
static int beep_count = 0;

void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

/* Печать помощи */
void print_help(const char *prog) {
    printf("Usage: %s [--help|-h]\n", prog);
    printf("Ctrl+C - beep\n");
    printf("Ctrl+\\ - show statistics and exit\n");
    printf("Max line length: 40, words auto-wrap\n");
}

/* Программа в raw-режиме, обработка символов */
int main(int argc, char *argv[]) {
    char buf[128];
    int len_buf;

    if (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    } else if (argc > 1) {
        fprintf(stderr, "Invalid arguments. Use --help or -h\n");
        return EXIT_FAILURE;
    }

    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        return EXIT_FAILURE;
    }
    atexit(restore_terminal);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO | ISIG); // raw mode, без обработки Ctrl+C/ Ctrl+Z терминалом
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1) {
        perror("tcsetattr");
        return EXIT_FAILURE;
    }

    printf("Program started. PID: %d\n", getpid());
    printf("Press Ctrl+C for beep\nPress Ctrl+\\ to show statistics and exit\n");
    printf("========================================\n");
    fflush(stdout);

    char line[LINE_LENGTH + 1] = {0};
    int line_len = 0;
    int word_start = 0;

    char c;
    while (1) {
        if (read(STDIN_FILENO, &c, 1) != 1) {
            if (errno == EINTR) continue;
            perror("read");
            return EXIT_FAILURE;
        }

        if (!isprint((unsigned char)c)) {
            switch (c) {
                case 3: // Ctrl+C
                    beep_count++;
                    if (write(STDOUT_FILENO, "\aBEEP!\n", 7) == -1) {
                        perror("write");
                        return EXIT_FAILURE;
                    }
                    break;
                case 28: 
                    len_buf = snprintf(buf, sizeof(buf),
                                       "\nProgram finished. Count of BEEP: %d\n",
                                       beep_count);
                    write(STDOUT_FILENO, buf, len_buf);
                    return EXIT_SUCCESS;
                case 127: // Backspace (Erase)
                    if (line_len > 0) {
                        line[--line_len] = 0;
                        write(STDOUT_FILENO, "\b \b", 3);
                    }
                    break;
                case 21: // Ctrl+U (Kill)
                    while (line_len > 0) {
                        write(STDOUT_FILENO, "\b \b", 3);
                        line[--line_len] = 0;
                    }
                    break;
                case 23: // Ctrl+W (Word erase)
                    while (line_len > 0 && line[line_len-1] == ' ') line_len--;
                    while (line_len > 0 && line[line_len-1] != ' ') line_len--;
                    line[line_len] = 0;
                    write(STDOUT_FILENO, "\r", 1);
                    write(STDOUT_FILENO, line, line_len);
                    break;
                default:
                    write(STDOUT_FILENO, "\a", 1); // beep для прочих непечатаемых
                    break;
            }
        } else { // печатный символ
            if (line_len == 0 || line[line_len - 1] == ' ')
                word_start = line_len;

            // если слово не помещается на текущей линии, переносим целиком
            if (line_len == LINE_LENGTH) {
                int word_len = line_len - word_start;
                write(STDOUT_FILENO, "\n", 1);
                memmove(line, line + word_start, word_len);
                line_len = word_len;
                word_start = 0;
            }

            line[line_len++] = c;
            line[line_len] = 0;
            if (write(STDOUT_FILENO, &c, 1) == -1) {
                perror("write");
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}