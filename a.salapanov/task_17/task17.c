#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>

#define MAX_COLS 40
#define CTRL(c) ((c) & 0x1f)

static struct termios orig_termios;

static void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static int find_last_word_start(const char *buf, int len) {
    int i = len - 1;
    while (i >= 0 && buf[i] == ' ')
        i--;
    if (i < 0)
        return -1;
    while (i >= 0 && buf[i] != ' ')
        i--;
    return i + 1;
}

static void erase_n_chars_on_screen(int n) {
    for (int i = 0; i < n; i++) {
        write(STDOUT_FILENO, "\b \b", 3);
    }
}

int main(void) {
    struct termios raw;
    char buf[MAX_COLS + 1];
    int len = 0;  

    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        return 1;
    }

    atexit(restore_terminal);

    raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        return 1;
    }

    // --- ПОДСКАЗКА ПОЛЬЗОВАТЕЛЮ ---
    printf("Строчный редактор (MAX_COLS = %d)\n", MAX_COLS);
    printf("Доступные сочетания клавиш:\n");
    printf("  Ctrl+D  — выход, если строка пуста; иначе — звуковой сигнал\n");
    printf("  ERASE   — стереть последний символ (Backspace)\n");
    printf("  KILL    — стереть всю строку (Ctrl+U)\n");
    printf("  Ctrl+W  — стереть последнее слово вместе с пробелами\n");
    printf("  Enter   — завершить строку и начать новую\n");
    printf("  Непечатаемые символы (кроме указанных) — звуковой сигнал\n");
    printf("\nНачинайте ввод...\n");
    fflush(stdout);
    // --- КОНЕЦ ПОДСКАЗКИ ---

    unsigned char erase_char = orig_termios.c_cc[VERASE];
    unsigned char kill_char  = orig_termios.c_cc[VKILL];

    unsigned char ch;
    ssize_t nread;

    while ((nread = read(STDIN_FILENO, &ch, 1)) == 1) {
        if (ch == CTRL('D')) {
            if (len == 0) {
                break;
            } else {
                char bell = '\a';
                write(STDOUT_FILENO, &bell, 1);
                continue;
            }
        }

        if (ch == erase_char) {
            if (len > 0) {
                len--;
                erase_n_chars_on_screen(1);
            } else {
                char bell = '\a';
                write(STDOUT_FILENO, &bell, 1);
            }
            continue;
        }

        if (ch == kill_char) {
            if (len > 0) {
                erase_n_chars_on_screen(len);
                len = 0;
            } else {
                char bell = '\a';
                write(STDOUT_FILENO, &bell, 1);
            }
            continue;
        }

        if (ch == CTRL('W')) {
            if (len == 0) {
                char bell = '\a';
                write(STDOUT_FILENO, &bell, 1);
                continue;
            }
            int old_len = len;
            while (len > 0 && buf[len - 1] == ' ')
                len--;
            while (len > 0 && buf[len - 1] != ' ')
                len--;

            int to_erase = old_len - len;
            if (to_erase > 0) {
                erase_n_chars_on_screen(to_erase);
            } else {
                char bell = '\a';
                write(STDOUT_FILENO, &bell, 1);
            }
            continue;
        }

        if (ch == '\n' || ch == '\r') {
            write(STDOUT_FILENO, "\r\n", 2);
            len = 0;
            continue;
        }

        if (isprint(ch)) {
            if (ch == ' ') {
                if (len >= MAX_COLS) {
                    write(STDOUT_FILENO, "\r\n", 2);
                    len = 0;
                }
                if (len < MAX_COLS) {
                    buf[len++] = ch;
                    write(STDOUT_FILENO, &ch, 1);
                }
            } else {
                if (len < MAX_COLS) {
                    buf[len++] = ch;
                    write(STDOUT_FILENO, &ch, 1);
                } else {
                    int ws = find_last_word_start(buf, len);
                    if (ws < 0) {
                        write(STDOUT_FILENO, "\r\n", 2);
                        len = 0;
                        buf[len++] = ch;
                        write(STDOUT_FILENO, &ch, 1);
                    } else {
                        int wlen = len - ws; 
                        char temp[MAX_COLS + 1];

                        memcpy(temp, &buf[ws], wlen);
                        temp[wlen] = ch;
                        int new_wlen = wlen + 1;
                        if (new_wlen > MAX_COLS)
                            new_wlen = MAX_COLS; 

                        erase_n_chars_on_screen(wlen);
                        len = ws;

                        write(STDOUT_FILENO, "\r\n", 2);
                        len = 0;

                        for (int i = 0; i < new_wlen; i++) {
                            buf[len++] = temp[i];
                            write(STDOUT_FILENO, &temp[i], 1);
                        }
                    }
                }
            }
            continue;
        }

        {
            char bell = '\a';
            write(STDOUT_FILENO, &bell, 1);
        }
    }

    return 0;
}
