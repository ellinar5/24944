#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "[ERROR] Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    printf("[LOG] Program started. Filename: %s (PID: %d)\n", filename, getpid());

    // Выбор части задания
    int choice;
    printf("Choose task part (1 = no wait, 2 = wait): ");
    scanf("%d", &choice);
    printf("[LOG] User chose part: %d (PID: %d)\n", choice, getpid());

    if (choice == 1) {
        printf("[INFO] Part 1: Parent prints text immediately, child runs cat in parallel.\n");
    } else if (choice == 2) {
        printf("[INFO] Part 2: Parent waits for child to finish before printing last line.\n");
    } else {
        printf("[ERROR] Invalid choice. Exiting.\n");
        return 1;
    }

    // Создание подпроцесса
    pid_t pid = fork();
    if (pid == -1) {
        perror("[ERROR] fork failed");
        return 1;
    }

    if (pid == 0) {
        // Дочерний процесс
        printf("[LOG] Child started. (PID: %d)\n", getpid());
        printf("[LOG] Child executing: cat %s\n", filename);

        execlp("cat", "cat", filename, NULL);

        perror("[ERROR] exec failed");
        exit(1);
    } 
    else {
        // Родитель
        printf("[LOG] Parent running. (PID: %d)\n", getpid());

        if (choice == 1) {
            // Часть 1 — без ожидания
            printf("[PARENT] Printing text while child is still running...\n");
            printf("[PARENT] Parent continues executing without waiting.\n");
        } 
        else {
            // Часть 2 — с ожиданием
            int status;
            waitpid(pid, &status, 0);

            printf("[LOG] Child finished. Exit status: %d (PID: %d)\n",
                   WIFEXITED(status) ? WEXITSTATUS(status) : -1, getpid());

            printf("[PARENT] Child process ended, now printing final line.\n");
        }

        printf("[LOG] Program completed. (PID: %d)\n", getpid());
    }

    return 0;
}
