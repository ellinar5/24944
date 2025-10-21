#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>

extern char **environ;

int main(int argc, char *argv[]) {
    char options[] = ":ispuU:cC:dvV:";
    int c;

    while ((c = getopt(argc, argv, options)) != EOF) {
        switch (c) {
            case 'i': {
                printf("uid: %d, euid: %d, gid: %d, egid: %d\n", getuid(), geteuid(), getgid(), getegid());
                break;
            }

            case 's': {
                if (setpgid(0, 0) == -1) {
                    perror("FAILED TO SET THE GROUP LEADER PROCESS");
                } 
                else {
                    printf("the group leader process has been set successfully\n");
                }

                break;
            }

            case 'p': {
                printf("pid: %d, ppid: %d, pgrp: %d\n", getpid(), getppid(), getpgrp());
                break;
            }

            case 'u': {
                struct rlimit rlp;
                if (getrlimit(RLIMIT_FSIZE, &rlp) == -1) {
                    perror("FAILED TO GET ULIMIT");
                } 
                else {
                        if (rlp.rlim_cur == RLIM_INFINITY) {
                            printf("ulimit value: unlimited\n");
                        }

                        else {
                            printf("ulimit value: %lld\n", (long long)rlp.rlim_cur);
                        }
                    }
                break;
            }

            case 'U': {
                char *endptr;
                long new_ulimit = strtol(optarg, &endptr, 10);
                
                if (endptr == optarg || *endptr != '\0' || new_ulimit < -1) {
                    fprintf(stderr, "Invalid argument for -U: %s\n", optarg);
                    break;
                }

                struct rlimit rlp;
                if (getrlimit(RLIMIT_FSIZE, &rlp) == -1) {
                    perror("FAILED TO GET THE ULIMIT VALUE");
                    break;
                }
                
                rlp.rlim_cur = (new_ulimit == -1) ? RLIM_INFINITY : (rlim_t)new_ulimit;
                
                if (setrlimit(RLIMIT_FSIZE, &rlp) == -1) {
                    perror("FAILED TO SET THE ULIMIT VALUE");
                }

                else {
                    printf("ulimit value has been set successfully\n");
                }

                break;
            }

            case 'c': {
                struct rlimit rlp;
                if (getrlimit(RLIMIT_CORE, &rlp) == -1) {
                    perror("FAILED TO GET THE CORE-FILE SIZE LIMIT");
                } 
                else {
                    if (rlp.rlim_cur == RLIM_INFINITY) {
                        printf("core-file size limit: unlimited\n");
                    } 
                    else {
                        printf("core-file size limit: %lld bytes\n", (long long)rlp.rlim_cur);
                    }
                }
                break;
            }

            case 'C': {

                char *endptr;
                long new_rlim = strtol(optarg, &endptr, 10);
                
                if (endptr == optarg || *endptr != '\0' || new_rlim < -1) {
                    fprintf(stderr, "Invalid argument for -C: %s\n", optarg);
                    break;
                }

                struct rlimit rlp;

                if (getrlimit(RLIMIT_CORE, &rlp) == -1) {
                    perror("FAILED TO GET THE CORE-FILE SIZE LIMIT");
                    break;
                }
                
                rlp.rlim_cur = (new_rlim == -1) ? RLIM_INFINITY : (rlim_t)new_rlim;
                
                if (setrlimit(RLIMIT_CORE, &rlp) == -1) {
                    perror("FAILED TO SET THE CORE-FILE SIZE LIMIT");
                }

                else {
                    printf("core-file size limit has been set successfully\n");
                }

                break;
            }

            case 'd': {
                char pathname[4096];

                if (getcwd(pathname, sizeof(pathname)) == NULL) {
                    perror("FAILED TO GET THE CURRENT DIRECTORY");
                }

                else {
                    printf("current directory: %s\n", pathname);
                }

                break;
            }

            case 'v': {
                for (char **ptr = environ; *ptr != NULL; ptr++) {
                    printf("%s\n", *ptr);
                }
                break;
            }

            case 'V': {
                if (putenv(optarg) != 0) {
                    perror("FAILED TO SET THE ENVIRONMENTAL VARIABLE");
                }

                else {
                    printf("environment variable set: %s\n", optarg);
                }
                break;
            }

            default:
                fprintf(stderr, "invalid option: -%c\n", optopt);
        }
    }

    return 0;
}