/*
============================================================================
Name        : server.c
Author      : Sam Lunev
Version     : .0
Copyright   : All rights reserved
Description : OSLrCPT30
============================================================================
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/uds_socket"
#define BUFFER_SIZE 1024

volatile sig_atomic_t running = 1;

void signal_handler(int sig) {
    running = 0;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    
    unlink(SOCKET_PATH);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 1) == -1) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on %s\n", SOCKET_PATH);
    
    while (running) {
        client_len = sizeof(client_addr);
        if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len)) == -1) {
            if (errno == EINTR) continue;
            perror("accept failed");
            break;
        }
        
        printf("Client connected\n");
        
        bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            
            for (int i = 0; i < bytes_received; i++) {
                if (buffer[i] >= 'a' && buffer[i] <= 'z') {
                    buffer[i] = buffer[i] - 'a' + 'A';
                }
            }
            
            printf("%s\n", buffer);
        } else if (bytes_received == 0) {
            printf("Client disconnected\n");
        } else {
            perror("recv failed");
        }
        
        close(client_fd);
        
        break;
    }
    
    close(server_fd);
    unlink(SOCKET_PATH);
    printf("Server stopped\n");
    
    return 0;
}
