#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void print_help(void) {
    printf("Usage: program [OPTION]\n");
    printf("Demonstrates piping using popen():\n");
    printf("  1) Runs command: echo \"Hello, World!\"\n");
    printf("  2) Sends output to: tr [:lower:] [:upper:]\n\n");
    printf("Options:\n");
    printf("  -h, --help     Display this help message\n");
}

int main(int argc, char *argv[]) {
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

    char line[BUFSIZ];

    FILE *fpin = popen("echo Hello, World!", "r");
    if (!fpin) {
        fprintf(stderr, "Error: popen() for input failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    FILE *fpout = popen("tr [:lower:] [:upper:]", "w");
    if (!fpout) {
        fprintf(stderr, "Error: popen() for output failed: %s\n", strerror(errno));
        pclose(fpin);
        return EXIT_FAILURE;
    }

    while (fgets(line, BUFSIZ, fpin) != NULL) {
        if (fputs(line, fpout) == EOF) {
            fprintf(stderr, "Error: fputs() failed: %s\n", strerror(errno));
            pclose(fpin);
            pclose(fpout);
            return EXIT_FAILURE;
        }
    }

    if (ferror(fpin)) {
        fprintf(stderr, "Error: fgets() failed while reading input\n");
        pclose(fpin);
        pclose(fpout);
        return EXIT_FAILURE;
    }

    if (pclose(fpin) == -1) {
        fprintf(stderr, "Error: pclose() (input) failed: %s\n", strerror(errno));
        pclose(fpout);
        return EXIT_FAILURE;
    }

    if (pclose(fpout) == -1) {
        fprintf(stderr, "Error: pclose() (output) failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
