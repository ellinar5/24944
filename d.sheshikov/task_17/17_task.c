#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#define MAX_LINE_LENGTH 40
#define BELL '\a'

static struct termios orig_term;

void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
}

void setup_terminal(void) {
    struct termios new_term;
    
    tcgetattr(STDIN_FILENO, &orig_term);
    atexit(restore_terminal);
    
    new_term = orig_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    new_term.c_cc[VMIN] = 1;
    new_term.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
}

void erase_chars(char *line, int *pos, int *column, int count) {
    while (count-- > 0 && *pos > 0) {
        (*pos)--;
        (*column)--;
        printf("\b \b");
        line[*pos] = '\0';
    }
    fflush(stdout);
}

void erase_word(char *line, int *pos, int *column) {
    int start_pos = *pos;
    int start_column = *column;
    

    while (*pos > 0 && (line[*pos - 1] == ' ' || line[*pos - 1] == '\t')) {
        (*pos)--;
        (*column)--;
    }

    while (*pos > 0 && !(line[*pos - 1] == ' ' || line[*pos - 1] == '\t')) {
        (*pos)--;
        (*column)--;
    }
    
    if (start_pos == *pos) {
        printf("%c", BELL);
    } else {
        for (int i = 0; i < start_column - *column; i++) {
            printf("\b \b");
        }

        for (int i = *pos; i < start_pos; i++) {
            line[i] = '\0';
        }
    }
    fflush(stdout);
}

int find_wrap_position(const char *line, int current_pos) {
    if (current_pos <= MAX_LINE_LENGTH) return -1;
    
    for (int i = MAX_LINE_LENGTH - 1; i >= 0; i--) {
        if (line[i] == ' ' || line[i] == '\t') {
            return i + 1;
        }
    }
    return MAX_LINE_LENGTH;
}

void perform_word_wrap(char *line, int *pos, int *column) {
    int wrap_pos = find_wrap_position(line, *pos);
    if (wrap_pos == -1) return;
    

    int current_column = *column;
    
    for (int i = 0; i < current_column; i++) {
        printf("\b \b");
    }
    
    for (int i = 0; i < wrap_pos; i++) {
        printf("%c", line[i]);
    }
    printf("\n");
    
    int remaining = *pos - wrap_pos;
    for (int i = 0; i < remaining; i++) {
        printf("%c", line[wrap_pos + i]);
    }
    
    for (int i = 0; i < remaining; i++) {
        line[i] = line[wrap_pos + i];
    }
    line[remaining] = '\0';
    
    *pos = remaining;
    *column = remaining;
    fflush(stdout);
}

int main(void) {
    char line[MAX_LINE_LENGTH * 2] = {0};
    int pos = 0, column = 0;
    char c;

    setup_terminal();

    printf("Line editor (max %d chars, word wrap)\n", MAX_LINE_LENGTH);
    printf("Backspace=del char, Ctrl+U=kill line, Ctrl+W=del word, Ctrl+D=exit\n");

    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == 0x7F || c == 0x08) { // Backspace
            erase_chars(line, &pos, &column, 1);
        }
        else if (c == 0x15) { // Ctrl+U
            erase_chars(line, &pos, &column, pos);
        }
        else if (c == 0x17) { // Ctrl+W
            erase_word(line, &pos, &column);
        }
        else if (c == 0x04) { // Ctrl+D
            if (pos == 0 && column == 0) {
                printf("\nExit...\n");
                break;
            }
            printf("%c", BELL);
        }
        else if (c == '\n' || c == '\r') { // Enter
            printf("\n");
            pos = column = 0;
            line[0] = '\0';
        }
        else if (c >= 32 && c <= 126) { // Printable char
            if (pos < MAX_LINE_LENGTH * 2 - 1) {
                line[pos++] = c;
                line[pos] = '\0';
                printf("%c", c);
                column++;
                
                if (column >= MAX_LINE_LENGTH) {
                    perform_word_wrap(line, &pos, &column);
                }
            } else {
                printf("%c", BELL);
            }
        }
        else {
            printf("%c", BELL);
        }
        fflush(stdout);
    }

    return 0;
}