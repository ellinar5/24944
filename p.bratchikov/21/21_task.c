#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

static struct termios orig_termios;
static int beep_count = 0;

void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

int main() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        return EXIT_FAILURE;
    }

    atexit(restore_terminal);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO | ISIG); // raw mode
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1) {
        perror("tcsetattr");
        return EXIT_FAILURE;
    }

    printf("Press Ctrl+C for BEEP, Ctrl+\\ to exit\n");
    fflush(stdout);

    char c;
    while (1) {
        if (read(STDIN_FILENO, &c, 1) != 1) {
            perror("read");
            return EXIT_FAILURE;
        }

        if (c == 3) { // Ctrl+C
            beep_count++;
            write(STDOUT_FILENO, "\aBEEP!\n", 7);
        } else if (c == 28) { // Ctrl+\
            char buf[64];
            int len = snprintf(buf, sizeof(buf),
                               "Program finished. Count of BEEP: %d\n",
                               beep_count);
            write(STDOUT_FILENO, buf, len);
            return EXIT_SUCCESS;
        } else {
            write(STDOUT_FILENO, &c, 1); // печатные символы
        }
    }
}