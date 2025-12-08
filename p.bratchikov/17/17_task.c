#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
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

static struct termios orig_termios;


void restoreTerminal() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}


void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }


    atexit(restoreTerminal);

    struct termios raw = orig_termios;


    raw.c_lflag &= ~(ECHO | ICANON);


    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;


    raw.c_iflag |= ICRNL;  
    raw.c_oflag |= OPOST; 

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

void print_help(const char *prog) {
    printf("This program reads input in raw mode with line editing.\n");
    printf("Usage: %s [--help|-h]\n", prog);
}

int main(int argc, char *argv[]) {

    if (argc > 1 && (strcmp(argv[1], "--help") == 0 ||
                     strcmp(argv[1], "-h") == 0)) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }

    enableRawMode();

    char c;
    static char line[LINE_LENGTH + 1];

    while (read(STDIN_FILENO, &c, 1) == 1) {
        int len = strlen(line);

        if (iscntrl((unsigned char)c) || !isprint((unsigned char)c)) {
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
                        if (line[i] != ' ' && prev == ' ')
                            word_start = i;
                        prev = line[i];
                    }
                    line[word_start] = 0;
                    printf("\33[%dD\33[K", len - word_start);
                    break;
                }

                case CEOF:
                    if (line[0] == 0) {
                        return EXIT_SUCCESS;  
                    }
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
            line[len]   = 0;
            putchar(c);
        }

        fflush(stdout);
    }

    return EXIT_SUCCESS;
}