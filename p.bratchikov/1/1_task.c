#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/resource.h>
#include <string.h>
#include <errno.h>

extern char **environ;

void print_help() {
    printf("Usage: %s [options]\n");
    printf("Options:\n");
    printf("  -i           Print UID, GID, EUID, EGID\n");
    printf("  -s           Set process as a group leader\n");
    printf("  -p           Print PID, PPID, PGRP\n");
    printf("  -u           Print file size ulimit\n");
    printf("  -U <n>       Set file size ulimit (use -1 for unlimited)\n");
    printf("  -c           Print core-file size limit\n");
    printf("  -C <n>       Set core-file size limit (use -1 for unlimited)\n");
    printf("  -d           Print current directory\n");
    printf("  -v           Print environment variables\n");
    printf("  -V <NAME=VALUE>  Set environment variable\n");
    printf("  -h, --help   Show help message\n");
}

int main(int argc, char *argv[]) {
    if (argc >= 2 && (strcmp(argv[1], "--help") == 0)) {
        print_help();
        return EXIT_SUCCESS;
    }

    char options[] = ":ispuU:cC:dvV:h";
    int c;

    while ((c = getopt(argc, argv, options)) != -1) {
        switch (c) {

            case 'h':
                print_help(argv[0]);
                return EXIT_SUCCESS;

            case 'i':
                printf("uid: %d, euid: %d, gid: %d, egid: %d\n",
                       getuid(), geteuid(), getgid(), getegid());
                break;

            case 's':
                if (setpgid(0, 0) == -1) {
                    fprintf(stderr, "Error: failed to set process group leader\n");
                    exit(EXIT_FAILURE);
                }
                printf("Group leader process has been set successfully\n");
                break;

            case 'p':
                printf("pid: %d, ppid: %d, pgrp: %d\n",
                       getpid(), getppid(), getpgrp());
                break;

            case 'u': {
                struct rlimit rlp;
                if (getrlimit(RLIMIT_FSIZE, &rlp) == -1) {
                    fprintf(stderr, "Error: failed to get file size ulimit\n");
                    exit(EXIT_FAILURE);
                }

                if (rlp.rlim_cur == RLIM_INFINITY)
                    printf("ulimit value: unlimited\n");
                else
                    printf("ulimit value: %lld\n", (long long)rlp.rlim_cur);
                break;
            }

            case 'U': {
                char *endptr;
                long val = strtol(optarg, &endptr, 10);

                if (endptr == optarg || *endptr != '\0' || val < -1) {
                    fprintf(stderr, "Error: invalid argument for -U: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }

                struct rlimit rlp;
                if (getrlimit(RLIMIT_FSIZE, &rlp) == -1) {
                    fprintf(stderr, "Error: failed to get file size ulimit\n");
                    exit(EXIT_FAILURE);
                }

                rlp.rlim_cur = (val == -1 ? RLIM_INFINITY : (rlim_t)val);

                if (setrlimit(RLIMIT_FSIZE, &rlp) == -1) {
                    fprintf(stderr, "Error: failed to set file size ulimit\n");
                    exit(EXIT_FAILURE);
                }

                printf("File size ulimit updated successfully\n");
                break;
            }

            case 'c': {
                struct rlimit rlp;
                if (getrlimit(RLIMIT_CORE, &rlp) == -1) {
                    fprintf(stderr, "Error: failed to get core-file limit\n");
                    exit(EXIT_FAILURE);
                }

                if (rlp.rlim_cur == RLIM_INFINITY)
                    printf("core-file size limit: unlimited\n");
                else
                    printf("core-file size limit: %lld\n", (long long)rlp.rlim_cur);
                break;
            }

            case 'C': {
                char *endptr;
                long val = strtol(optarg, &endptr, 10);

                if (endptr == optarg || *endptr != '\0' || val < -1) {
                    fprintf(stderr, "Error: invalid argument for -C: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }

                struct rlimit rlp;
                if (getrlimit(RLIMIT_CORE, &rlp) == -1) {
                    fprintf(stderr, "Error: failed to get core-file limit\n");
                    exit(EXIT_FAILURE);
                }

                rlp.rlim_cur = (val == -1 ? RLIM_INFINITY : (rlim_t)val);

                if (setrlimit(RLIMIT_CORE, &rlp) == -1) {
                    fprintf(stderr, "Error: failed to set core-file limit\n");
                    exit(EXIT_FAILURE);
                }

                printf("Core-file size limit updated successfully\n");
                break;
            }

            case 'd': {
                char pathname[4096];
                if (getcwd(pathname, sizeof(pathname)) == NULL) {
                    fprintf(stderr, "Error: failed to get current directory\n");
                    exit(EXIT_FAILURE);
                }
                printf("Current directory: %s\n", pathname);
                break;
            }

            case 'v':
                for (char **ptr = environ; *ptr; ptr++)
                    printf("%s\n", *ptr);
                break;

            case 'V':
                if (putenv(optarg) != 0) {
                    fprintf(stderr, "Error: failed to set environment variable\n");
                    exit(EXIT_FAILURE);
                }
                printf("Environment variable set: %s\n", optarg);
                break;

            case ':':
                fprintf(stderr, "Error: option -%c requires argument\n", optopt);
                exit(EXIT_FAILURE);

            case '?':
            default:
                fprintf(stderr, "Error: invalid option -%c\n", optopt);
                exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}