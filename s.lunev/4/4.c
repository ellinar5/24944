/*
 ============================================================================
 Name        : 4.c
 Author      : Sam Lunev
 Version     : .0
 Copyright   : All rights reserved
 Description : OSLrCrsT4
 ============================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 512

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

void delay(int);
void print_list(Node *head);
void free_list(Node *head);

int main() {

    char buffer[BUFFER_SIZE];
    Node *head = NULL;
    Node **current = &head;
    
    printf("Enter sentences (enter a period (.) at the beginning to stop):\n");
    while(1){
        printf("> ");
        memset(buffer, 0, BUFFER_SIZE);

        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            break;
        }
        
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }
        
        if (len > 0 && buffer[0] == '.') {
            break;
        }
        
        Node *new_node = malloc(sizeof(struct Node));
        new_node->data = malloc(len + 1);
        
        strcpy(new_node->data, buffer);
        new_node->next = NULL;
        
        *current = new_node;
        current = &(*current)->next;
        delay(1);
    }
    print_list(head);
    free_list(head);
    
    return 0;
}

void delay(int n) {
    int td = 10*n;
    
    clock_t s_t = clock();
    
    while (clock() < s_t + td)
        ;
}

void print_list(Node *head) {
    Node *current = head;
    int count = 1;
    
    if (current == NULL) {
        printf("List is empty\n");
        return;
    }
    
    while (current != NULL) {
        printf("%d: %s\n", count, current->data);
        current = current->next;
        count++;
    }
}

void free_list(Node *head) {
    Node *current = head;
    Node *next;
    
    while (current != NULL) {
        next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
}
