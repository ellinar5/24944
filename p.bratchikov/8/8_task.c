#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void print_help() {
    printf("File Locker Editor\n");
    printf("This program attempts to exclusively lock a file and open it in vim.\n");
    printf("If the file is locked by another process, it reports the PID.\n\n");

    printf("Usage:\n");
    printf("  program <filename>\n");
    printf("  program -h\n");
    printf("  program --help\n\n");

    printf("Arguments:\n");
    printf("  <filename>   Name of the file to open and lock.\n");
    printf("  -h, --help   Show this help message and exit.\n");
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Error: exactly 1 argument required.\n");
        fprintf(stderr, "Usage: program <filename> OR program -h\n");
        return EXIT_FAILURE;
    }

    if ((strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0)) {
        print_help();
        return EXIT_SUCCESS;
    }

    char *filename = argv[1];
    int fd = open(filename, O_RDWR);

    if (fd == -1) {
        fprintf(stderr, "Error: cannot open file '%s': %s\n", filename, strerror(errno));
        return EXIT_FAILURE;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;

    if (fcntl(fd, F_SETLK, &lock) == -1) {

        struct flock info;
        info.l_type = F_WRLCK;
        info.l_start = 0;
        info.l_whence = SEEK_SET;
        info.l_len = 0;

        if (fcntl(fd, F_GETLK, &info) == -1) {
            fprintf(stderr, "Error: fcntl(F_GETLK) failed: %s\n", strerror(errno));
            close(fd);
            return EXIT_FAILURE;
        }

        if (info.l_type == F_UNLCK) {
            fprintf(stderr, "Error: locking failed, but file is reported as unlocked.\n");
        } else {
            fprintf(stderr, "Error: the file '%s' is locked by PID %d.\n", filename, info.l_pid);
        }

        close(fd);
        return EXIT_FAILURE;
    }

    char cmd[FILENAME_MAX * 2] = "nano ";
    if (strlen(filename) >= FILENAME_MAX) {
        fprintf(stderr, "Error: filename is too long.\n");
        close(fd);
        return EXIT_FAILURE;
    }

    strcat(cmd, filename);

    int sysret = system(cmd);
    if (sysret == -1) {
        fprintf(stderr, "Error: failed to run nano: %s\n", strerror(errno));
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        return EXIT_FAILURE;
    }

    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &lock) == -1) {
        fprintf(stderr, "Error: failed to unlock file: %s\n", strerror(errno));
        close(fd);
        return EXIT_FAILURE;
    }

    if (close(fd) == -1) {
        fprintf(stderr, "Error: failed to close file: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}