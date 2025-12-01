#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void print_help(const char *progname) {
    printf("This program displays real/effective UID, "
           "changes EUID to real UID using setuid(), "
           "and tests opening a file before and after.\n\n");
    printf("Usage:\n");
    printf("  %s <filename>\n", progname);
    printf("  %s --help | -h\n", progname);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}

void UID_show(const char* st) {
    printf("%s:\n", st);
    printf("REAL UID: %d | EFFECTIVE UID: %d\n", getuid(), geteuid());
}

void Open(const char *file_a) {
    printf("Trying to open: %s\n", file_a);

    FILE *file = fopen(file_a, "r");
    if (file == NULL) {
        fprintf(stderr, "ERROR: Failed to open file '%s': %s\n",
                file_a, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (fclose(file) != 0) {
        fprintf(stderr, "ERROR: fclose() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "ERROR: Invalid number of arguments.\n");
        fprintf(stderr, "Usage: %s <filename> or %s --help\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }


    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_help(argv[0]);
        exit(EXIT_SUCCESS);
    }

    if (argv[1][0] == '-' && argv[1][1] == '-') {
        fprintf(stderr, "ERROR: Unknown option '%s'.\n", argv[1]);
        fprintf(stderr, "Use --help for usage information.\n");
        exit(EXIT_FAILURE);
    }

    UID_show("BEFORE setuid()");
    Open(argv[1]);

    if (setuid(getuid()) == -1) {
        fprintf(stderr, "ERROR: setuid() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    UID_show("AFTER setuid()");
    Open(argv[1]);

    return EXIT_SUCCESS;
}