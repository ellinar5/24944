#include <unistd.h>
#include <sys/termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/ttydefaults.h>
#include <string.h>

#ifndef CERASE
#define CERASE 0x7F
#endif

#ifndef CKILL
#define CKILL 0x15
#endif

#ifndef CWERASE
#define CWERASE 0x17
#endif

#ifndef CEOF
#define CEOF 0x04
#endif

#define LINE_LENGTH 40
struct termios orig_termios;

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        fprintf(stderr, "tcsetattr failed\n");
        exit(EXIT_FAILURE);
    }
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        fprintf(stderr, "tcgetattr failed\n");
        exit(EXIT_FAILURE);
    }
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        fprintf(stderr, "tcsetattr failed\n");
        exit(EXIT_FAILURE);
    }
}

void print_help(const char *prog) {
    printf("This program reads input in raw mode with line editing.\n");
    printf("Usage: %s [--help|-h]\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }

    enableRawMode();

    char c;
    static char line[LINE_LENGTH + 1];

    while (read(STDIN_FILENO, &c, 1) == 1) {
        int len = strlen(line);
        if (iscntrl(c) || !isprint(c)) {
            switch (c) {
                case CERASE:
                    if (len > 0) {
                        line[len - 1] = 0;
                        printf("\33[D\33[K");
                    }
                    break;

                case CKILL:
                    line[0] = 0;
                    printf("\33[2K\r");
                    break;

                case CWERASE: {
                    int word_start = 0;
                    char prev = ' ';
                    for (int i = 0; i < len; i++) {
                        if (line[i] != ' ' && prev == ' ') {
                            word_start = i;
                        }
                        prev = line[i];
                    }
                    line[word_start] = 0;
                    printf("\33[%dD\33[K", len - word_start);
                    break;
                }

                case CEOF:
                    if (line[0] == 0) { exit(EXIT_SUCCESS); }
                    break;

                default:
                    putchar('\a');
                    break;
            }
        } else {
            if (len == LINE_LENGTH) {
                putchar('\n');
                len = 0;
            }
            line[len++] = c;
            line[len] = 0;
            putchar(c);
        }
        fflush(NULL);
    }

    return EXIT_SUCCESS;
}