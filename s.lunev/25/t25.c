/*
 ============================================================================
 Name        : t25.c
 Author      : Sam Lunev
 Version     : .0
 Copyright   : All rights reserved
 Description : OSLrnCrsPT25
 ============================================================================
 */

 #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <errno.h>

#define BUFFER_SIZE 1024

int setup_pipe(int pipefd[2]);
pid_t create_child_process();
void child_process(int pipefd[2]);
void parent_process(int pipefd[2]);
void safe_write(int fd, const char *buffer, size_t length);
void safe_close(int fd);

int main(void)
{
    int pipefd[2];
    pid_t pid;
    
    if (setup_pipe(pipefd) == -1) {
        fprintf(stderr, "Failed to create pipe\n");
        return EXIT_FAILURE;
    }
    
    pid = create_child_process();
    if (pid == -1) {
        perror("fork");
        safe_close(pipefd[0]);
        safe_close(pipefd[1]);
        return EXIT_FAILURE;
    }
    
    if (pid == 0) {
          child_process(pipefd);
    } else {
          parent_process(pipefd);
    }
    
    return 0;
}

int setup_pipe(int pipefd[2])
{
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }
    return 0;
}

pid_t create_child_process()
{
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
    }
    return pid;
}

void child_process(int pipefd[2])
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    // Close write end of pipe
    safe_close(pipefd[1]);
    
    // Read from pipe and process data
    while ((bytes_read = read(pipefd[0], buffer, BUFFER_SIZE - 1)) > 0) {
        // Validate read result
        if (bytes_read < 0) {
            perror("read");
            break;
        }
        
        buffer[bytes_read] = '\0';
        
        // Convert to uppercase
        for (int i = 0; i < bytes_read; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        
        printf("Child process: %s", buffer);
        fflush(stdout);
    }
    
    safe_close(pipefd[0]);
}

void parent_process(int pipefd[2])
{
    char *messages[] = {
        "C!!!\n",
        "Solaris !!!!\n",
        "Linux!!!\n"
    };
    
    int num_messages = sizeof(messages) / sizeof(messages[0]);
    
    safe_close(pipefd[0]);
    
    for (int i = 0; i < num_messages; i++) {
        if (messages[i] == NULL) {
            fprintf(stderr, "Error: Null message at index %d\n", i);
            continue;
        }
        
        size_t message_len = strlen(messages[i]);
        if (message_len == 0) {
            fprintf(stderr, "Warning: Empty message at index %d\n", i);
            continue;
        }
        
        printf("The parent send a message: %s", messages[i]);
        safe_write(pipefd[1], messages[i], message_len);
        sleep(3);
    }
    
    safe_close(pipefd[1]);
    wait(NULL);
    printf("The parent process is completed.\n");
}

void safe_write(int fd, const char *buffer, size_t length)
{
    if (buffer == NULL || length == 0) {
        return;
    }
    
    ssize_t bytes_written = write(fd, buffer, length);
    if (bytes_written == -1) {
        perror("write");
    } else if ((size_t)bytes_written != length) {
        fprintf(stderr, "Warning: Only %zd of %zu bytes written\n", bytes_written, length);
    }
}

void safe_close(int fd)
{
    if (close(fd) == -1) {
        if (errno != EBADF) {
            perror("close");
        }
    }
}
