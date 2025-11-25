#include <unistd.h>
#include <sys/termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define LINE_LENGTH 40
struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
    enableRawMode();

    char c;
    static char line[LINE_LENGTH + 1];

    while (read(STDIN_FILENO, &c, 1) == 1) {
        int len = strlen(line);
        if (iscntrl(c) || !isprint(c)) {
            switch (c) {
                case 0x7F: {  // CERASE - Backspace/Delete
                    // Когда вводится символ ERASE, стирается
                    // последний символ в текущей строке.
                    if (len > 0) {
                        line[len - 1] = 0;
                        // [D - Move cursor left one char
                        // [K - Clear line from cursor right
                        printf("\33[D\33[K");
                    }
                    break;
                }

                case 0x15: {  // CKILL - Ctrl+U
                    // Когда вводится символ KILL, стираются
                    // все символы в текущей строке.
                    line[0] = 0;
                    // [2K - Clear entire line
                    printf("\33[2K\r");
                    break;
                }

                case 0x17: {  // CWERASE - Ctrl+W
                    // Когда вводится CTRL-W, стирается последнее слово в текущей
                    // строке, вместе со всеми следующими за ним пробелами.
                    if (len > 0) {
                        int word_start = len;
                        char prev = ' ';
                        for (int i = len - 1; i >= 0; i--) {
                            if (line[i] != ' ' && prev == ' ') {
                                word_start = i;
                            } else if (line[i] == ' ' && prev != ' ') {
                                break;
                            }
                            prev = line[i];
                        }

                        line[word_start] = 0;
                        // [<n>D - Move cursor left n chars
                        // [K - Clear line from cursor right
                        printf("\33[%dD\33[K", len - word_start);
                    }
                    break;
                }

                case 0x04: {  // CEOF - Ctrl+D
                    // Программа завершается, когда введен CTRL-D
                    // и курсор находится в начале строки.
                    if (line[0] == 0) { exit(0); }
                    break;
                }

                default: {
                    // Все непечатаемые символы, кроме перечисленных выше, должны
                    // издавать звуковой сигнал, выводя на терминал символ CTRL-G.
                    putchar('\a');
                    break;
                }
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

    return 0;
}