#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_LINE 40
#define BELL '\a'
#define CTRL(c) ((c) & 0x1F)

#define ERASE       0x7F
#define KILL        CTRL('U')
#define WORD_ERASE  CTRL('W')
#define EOF_KEY     CTRL('D')

// Глобальные переменные
struct termios orig_termios;
char line[1024] = {0}; // буфер для всей строки
int pos = 0;            // текущая позиция в буфере
int cur_col = 0;        // текущий столбец на экране

void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode(void) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(restore_terminal);

    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag |= CS8;
    raw.c_oflag &= ~(OPOST);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void beep(void) {
    write(STDOUT_FILENO, "\a", 1);
}

// Удаление последнего слова
void erase_word(void) {
    while (pos > 0 && line[pos-1] == ' ') {
        pos--; cur_col--;
        write(STDOUT_FILENO, "\b \b", 3);
    }
    while (pos > 0 && line[pos-1] != ' ') {
        pos--; cur_col--;
        write(STDOUT_FILENO, "\b \b", 3);
    }
}

// Перенос строки по словам
void wrap_line(void) {
    int last_space = -1;
    for (int i = pos - 1; i >= 0; i--) {
        if (line[i] == ' ') {
            last_space = i;
            break;
        }
    }

    if (last_space == -1) {
        // Нет пробела, просто перенос всей строки
        write(STDOUT_FILENO, "\r\n", 2);
        cur_col = pos;
    } else {
        // Переносим слово на новую строку
        int move_len = pos - last_space - 1;
        write(STDOUT_FILENO, "\r\n", 2);
        write(STDOUT_FILENO, line + last_space + 1, move_len);
        memmove(line, line + last_space + 1, move_len);
        pos = move_len;
        cur_col = move_len;
    }
}

int main(void) {
    enable_raw_mode();
    printf("CTRL+D в начале строки - выход\nCTRL+U - стирает всю строку\nCTRL+W - стирает последнее слово\n");
    printf("Введите текст:\n> ");

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == EOF_KEY && pos == 0) {
            printf("\n[LOG] Выход по Ctrl+D\n");
            break;
        }

        if (c == ERASE && pos > 0) {
            pos--; cur_col--;
            line[pos] = '\0';
            write(STDOUT_FILENO, "\b \b", 3);
            continue;
        }

        if (c == KILL) {
            while (pos > 0) {
                write(STDOUT_FILENO, "\b \b", 3);
                pos--; cur_col--;
            }
            line[0] = '\0';
            continue;
        }

        if (c == WORD_ERASE) {
            erase_word();
            line[pos] = '\0';
            continue;
        }

        if (c == '\r' || c == '\n') {
            write(STDOUT_FILENO, "\r\n", 2);
            write(STDOUT_FILENO, line, pos);
            write(STDOUT_FILENO, "\r\n> ", 4);
            pos = 0;
            cur_col = 0;
            line[0] = '\0';
            continue;
        }

        if (isprint(c)) {
            line[pos++] = c;
            write(STDOUT_FILENO, &c, 1);
            cur_col++;

            if (cur_col >= MAX_LINE) {
                wrap_line();
            }
        } else {
            beep();
        }
    }

    restore_terminal();
    return 0;
}
