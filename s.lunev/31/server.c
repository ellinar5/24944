/*
============================================================================
Name        : server.c
Author      : Sam Lunev
Version     : .0
Copyright   : All rights reserved
Description : OSLrCPT31
============================================================================
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define SOCKET_PATH "/tmp/uds_socket"
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_MESSAGE_LENGTH 100

volatile sig_atomic_t running = 1;

void signal_handler(int sig) {
    running = 0;
}

int main() {
    int server_fd, client_fd, max_fd;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_len;
    fd_set read_fds;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    int client_sockets[MAX_CLIENTS];
    int num_clients = 0;
    int i, j;
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = -1;
    }
    
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
    
    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on %s\n", SOCKET_PATH);
    
    while (running) {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        max_fd = server_fd;
        
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1) {
                FD_SET(client_sockets[i], &read_fds);
                if (client_sockets[i] > max_fd) {
                    max_fd = client_sockets[i];
                }
            }
        }
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        
        if (activity < 0) {
            if (errno == EINTR) continue;
            perror("select failed");
            break;
        }
        
        if (FD_ISSET(server_fd, &read_fds)) {
            client_len = sizeof(client_addr);
            if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len)) != -1) {
                // Find empty slot for new client
                int slot_found = 0;
                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (client_sockets[i] == -1) {
                        client_sockets[i] = client_fd;
                        num_clients++;
                        printf("New client connected, total clients: %d\n", num_clients);
                        slot_found = 1;
                        break;
                    }
                }
                
                if (!slot_found) {
                    printf("Max clients reached, rejecting new connection\n");
                    close(client_fd);
                }
            } else {
                perror("accept failed");
            }
        }
        
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1 && FD_ISSET(client_sockets[i], &read_fds)) {
                bytes_received = recv(client_sockets[i], buffer, BUFFER_SIZE - 1, 0);
                
                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';
                    
                    for (int j = 0; j < bytes_received; j++) {
                        if (buffer[j] >= 'a' && buffer[j] <= 'z') {
                            buffer[j] = buffer[j] - 'a' + 'A';
                        }
                    }
                    
                    printf("Received from client %d: %s\n", i, buffer);
                } else if (bytes_received == 0) {
                    printf("Client %d disconnected\n", i);
                    close(client_sockets[i]);
                    client_sockets[i] = -1;
                    num_clients--;
                } else {
                    perror("recv failed");
                    close(client_sockets[i]);
                    client_sockets[i] = -1;
                    num_clients--;
                }
            }
        }
    }
    
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != -1) {
            close(client_sockets[i]);
        }
    }
    close(server_fd);
    unlink(SOCKET_PATH);
    printf("Server stopped\n");
    
    return 0;
}
