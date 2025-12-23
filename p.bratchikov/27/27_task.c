#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void print_help(void) {
    printf("Usage: program [FILE]\n");
    printf("Counts the number of empty lines in FILE using grep and wc.\n");
    printf("Options:\n");
    printf("  -h, --help   Display this help message\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Error: missing filename\n");
        fprintf(stderr, "Use --help for usage.\n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help();
        return EXIT_SUCCESS;
    }

    char *filename = argv[1];
    char cmd[BUFSIZ];

    if (snprintf(cmd, BUFSIZ, "grep -c '^$' '%s'", filename) >= BUFSIZ) {
        fprintf(stderr, "Error: filename too long\n");
        return EXIT_FAILURE;
    }

    FILE *fp = popen(cmd, "r");
    if (!fp) {
        fprintf(stderr, "Error: popen failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    char line[BUFSIZ];
    if (!fgets(line, BUFSIZ, fp)) {
        fprintf(stderr, "Error: fgets failed or file empty\n");
        pclose(fp);
        return EXIT_FAILURE;
    }

    if (pclose(fp) == -1) {
        fprintf(stderr, "Error: pclose failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    printf("Number of empty lines: %s", line);

    return EXIT_SUCCESS;
}