#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define CERASE  0x7F
#define CKILL   0x15
#define CWERASE 0x17
#define CEOF    0x04

#define LINE_LENGTH 40

static struct termios orig;

void restore(void) {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig) == -1) {
        perror("tcsetattr restore");
        exit(EXIT_FAILURE);
    }
}

void raw_mode(void) {
    if (tcgetattr(STDIN_FILENO, &orig) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    atexit(restore);

    struct termios t = orig;
    t.c_lflag &= ~(ECHO | ICANON);
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &t) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

void print_help(const char *prog) {
    printf("Usage: %s [--help|-h]\n", prog);
    printf("Reads input in raw mode with line editing:\n");
    printf("  Backspace        - erase last character\n");
    printf("  Ctrl-U           - erase entire line\n");
    printf("  Ctrl-W           - erase last word + following spaces\n");
    printf("  Ctrl-D at line start - exit program\n");
    printf("  Other non-printable - beep (Ctrl-G)\n");
    printf("  Max line length: 40 characters; words longer than remaining space are moved to next line\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Error: unknown argument '%s'\n", argv[1]);
            fprintf(stderr, "Use --help or -h for usage\n");
            return EXIT_FAILURE;
        }
    }

    raw_mode();

    char line[LINE_LENGTH + 1] = {0};
    int len = 0;
    int word_start = 0;

    char c;
    while (1) {
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n == -1) {
            perror("read");
            return EXIT_FAILURE;
        } else if (n == 0) {
            continue;
        }

        if (isprint((unsigned char)c)) {

            if (len == 0 || line[len - 1] == ' ')
                word_start = len;

            line[len++] = c;
            line[len] = 0;
            putchar(c);

            if (len > LINE_LENGTH) {
                int word_len = len - word_start;

                
                for (int i = 0; i < word_len; i++)
                    printf("\b \b");

                putchar('\n');

                
                memmove(line, line + word_start, word_len);
                line[word_len] = 0;
                printf("%s", line);

                len = word_len;
                word_start = 0;
            }

        } else {
            switch (c) {

                case CERASE:
                    if (len > 0) {
                        line[--len] = 0;
                        printf("\b \b");
                    }
                    break;

                case CKILL:
                    while (len > 0)
                        printf("\b \b"), len--;
                    line[0] = 0;
                    break;

                case CWERASE:
                    while (len > 0 && line[len - 1] == ' ')
                        printf("\b \b"), len--;
                    while (len > 0 && line[len - 1] != ' ')
                        printf("\b \b"), len--;
                    line[len] = 0;
                    break;

                case CEOF:
                    if (len == 0)
                        return EXIT_SUCCESS;
                    break;

                default:
                    putchar('\a');
            }
        }

        fflush(stdout);
    }

    return EXIT_SUCCESS;
}