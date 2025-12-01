#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 1024

typedef struct list_element {
    char *text;
    struct list_element *next_element;
} list_element;

void print_help(const char *progname) {
    printf("This program reads lines from stdin until a line starting with '.' is entered.\n");
    printf("All entered lines are stored in a linked list and printed at the end.\n\n");
    printf("Usage:\n");
    printf("  %s          - run normally\n", progname);
    printf("  %s --help   - show this help\n", progname);
    printf("  %s -h       - show this help\n", progname);
}

int main(int argc, char *argv[]) {
    if (argc == 2) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        }
        else {
            fprintf(stderr, "Error: unknown option '%s'. Use --help.\n", argv[1]);
            exit(EXIT_FAILURE);
        }
    }
    else if (argc > 2) {
        fprintf(stderr, "Error: too many arguments. Use --help.\n");
        exit(EXIT_FAILURE);
    }


    list_element *first = NULL, *last = NULL;
    char user_input[INPUT_BUFFER_SIZE];

    printf("Type your text lines (enter a line starting with dot to stop):\n");

    while (fgets(user_input, INPUT_BUFFER_SIZE, stdin) != NULL) {
        size_t input_length = strlen(user_input);
        if (input_length > 0 && user_input[0] == '.') {
            break;
        }

        if (input_length > 0 && user_input[input_length - 1] == '\n') {
            user_input[input_length - 1] = '\0';
            input_length--;
        }


        char *text_storage = malloc(input_length + 1);
        if (text_storage == NULL) {
            fprintf(stderr, "Error: failed to allocate memory for text.\n");
            exit(EXIT_FAILURE);
        }
        strcpy(text_storage, user_input);


        list_element *new_item = malloc(sizeof(list_element));
        if (new_item == NULL) {
            fprintf(stderr, "Error: failed to allocate memory for list element.\n");
            free(text_storage);
            exit(EXIT_FAILURE);
        }

        new_item->text = text_storage;
        new_item->next_element = NULL;

        if (first == NULL) {
            first = new_item;
            last = new_item;
        } 
        else {
            last->next_element = new_item;
            last = new_item;
        }
    }


    printf("\nYour entered lines:\n");
    for (list_element *cur = first; cur != NULL; cur = cur->next_element) {
        printf("%s\n", cur->text);
    }

    list_element *cur = first;
    while (cur != NULL) {
        list_element *next = cur->next_element;
        free(cur->text);
        free(cur);
        cur = next;
    }

    return EXIT_SUCCESS;
}
