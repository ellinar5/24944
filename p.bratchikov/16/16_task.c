#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

void print_help(void) {
    printf("Usage: program [OPTION]\n");
    printf("Reads characters in noncanonical mode until 'q' is pressed.\n\n");
    printf("Options:\n");
    printf("  -h, --help      Display this help message\n");
}

int main(int argc, char *argv[]) {
    struct termios tty, savetty;
    int have_savetty = 0;

    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_help();
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Error: unknown option '%s'\n", argv[1]);
            fprintf(stderr, "Use --help for usage information.\n");
            return EXIT_FAILURE;
        }
    }

    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        fprintf(stderr, "Error: tcgetattr failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    savetty = tty;
    have_savetty = 1;

    tty.c_lflag &= ~(ECHO | ICANON);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        fprintf(stderr, "Error: tcsetattr (set mode) failed: %s\n", strerror(errno));
        /* пытаться восстановить только если есть сохранённые настройки */
        if (have_savetty) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &savetty);
        }
        return EXIT_FAILURE;
    }

    printf("Enter a symbol! (press 'q' to quit)\n");

    char c;
    ssize_t r;

    while ((r = read(STDIN_FILENO, &c, 1)) == 1) {
        if (c == 'q') break;
        if (printf("You entered: %c\n", c) < 0) {
            fprintf(stderr, "Error: printf failed\n");
            if (have_savetty) tcsetattr(STDIN_FILENO, TCSAFLUSH, &savetty);
            return EXIT_FAILURE;
        }
    }

    if (r == -1) {
        fprintf(stderr, "Error: read failed: %s\n", strerror(errno));
        if (have_savetty) tcsetattr(STDIN_FILENO, TCSAFLUSH, &savetty);
        return EXIT_FAILURE;
    }

    if (have_savetty) {
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &savetty) == -1) {
            fprintf(stderr, "Error: tcsetattr (restore mode) failed: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
