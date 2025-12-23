/*
============================================================================
Name        : client.c
Author      : Sam Lunev
Version     : .0
Copyright   : All rights reserved
Description : OSLrCPT32
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
#include <time.h>

#define SOCKET_PATH "/tmp/uds_socket"

int main(int argc, char *argv[]) {
    int client_fd;
    struct sockaddr_un server_addr;
    char *message;
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s \"message to send\"\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    message = argv[1];
    
    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("Connected to server\n");
    
    if (send(client_fd, message, strlen(message), 0) == -1) {
        perror("send failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("Message sent: %s\n", message);
    
    close(client_fd);
    printf("Client finished\n");
    
    return 0;
}

