#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <time.h>

void print_help(void) {
    printf("Usage: program\n");
    printf("Generates 100 random numbers (0-99), sorts them, and prints 10 numbers per line.\n");
    printf("Sort is performed via 'sort -n' using p2open/p2close.\n");
    printf("Options:\n");
    printf("  -h, --help   Display this help message\n");
}

int main(int argc, char* argv[]) {

    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_help();
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Error: unknown option '%s'\n", argv[1]);
            fprintf(stderr, "Use --help for usage.\n");
            return EXIT_FAILURE;
        }
    }

    FILE *fp[2];

    if (p2open("sort -n", fp) == -1) { return 1; }

    srand(time(NULL));
    for (int i = 0; i < 100; i++) {
        fprintf(fp[0], "%d\n", rand() % 100);
    }

    fclose(fp[0]);

    char c;
    int count = 0;
    while (read(fileno(fp[1]), &c, 1) == 1) {
        if (c == '\n') {
            printf(++count % 10 == 0 ? "\n" : " ");
            continue;
        }

        printf("%c", c);
    }

    p2close(fp);

    return 0;
}