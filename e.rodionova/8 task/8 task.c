#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int fd;
    struct flock lock;

    if (argc != 2) {
        fprintf(stderr, "Использование: %s <file>\n", argv[0]);
        return 1;
    }

    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;     /* блокировка на запись */
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;           /* весь файл */

    printf("Попытка захватить файл...\n");

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("fcntl");
        close(fd);
        return 1;
    }

    printf("Файл успешно захвачен.\n");
    printf("Запуск редактора...\n");

    system("vi");

    printf("Редактор закрыт. Освобождаем файл.\n");

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    close(fd);
    return 0;
}
