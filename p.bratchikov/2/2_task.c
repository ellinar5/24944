#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

void print_help(const char *prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("Options:\n");
    printf("  -h, --help       Show this help message\n");
    printf("The program prints the current time in PST8PDT time zone.\n");
}

int main(int argc, char *argv[])
{
    if (argc >= 2 && strcmp(argv[1], "--help") == 0 || argc >= 2 && strcmp(argv[1], "-h") == 0)
    {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }

    
    if (putenv("TZ=PST8PDT") != 0)
    {
        fprintf(stderr, "Error: failed to set TZ environment variable\n");
        exit(EXIT_FAILURE);
    }

    time_t now = time(NULL);
    if (now == (time_t)-1)
    {
        fprintf(stderr, "Error: failed to get current time\n");
        exit(EXIT_FAILURE);
    }

    char *str_time = ctime(&now);
    if (str_time == NULL)
    {
        fprintf(stderr, "Error: failed to convert time to string\n");
        exit(EXIT_FAILURE);
    }

    printf("%s", str_time);

    return EXIT_SUCCESS;
}
