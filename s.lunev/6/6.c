/*
 ============================================================================
 Name        : 6.c
 Author      : Sam Lunev
 Version     : .0
 Copyright   : All rights reserved
 Description : OSLrnCrsPT5
 ============================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>

#define MAX_LINE_LENGTH 1024
#define MAX_LINES 1000
#define TIMEOUT_SECONDS 5

typedef struct {
    int indent_level;
    int line_length;
    off_t file_offset;
} LineTable;

// Global variables for signal handling
static jmp_buf jump_buffer;
static int timeout_occurred = 0;
static int fl = 0;

// Function prototypes
int get_line_number_with_timeout(int max_lines, int fd);
void build_line_table(int fd, LineTable *table, int *line_count);
void print_line_table(LineTable *table, int line_count);
void read_and_print_line(int fd, LineTable *table, int line_num);
void alarm_handler(int sig);
int get_char(void);
int kbhit(void);
void clear_screen(void);

int main() {
    int fd;
    char filename[256];
    LineTable table[MAX_LINES];
    int line_count = 0;
    int line_num;
    
    printf("Enter text file name: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        fprintf(stderr, "Error reading filename\n");
        return 1;
    }
    filename[strcspn(filename, "\n")] = '\0';
    
    // Open file
    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return 1;
    }
    
    // Build line table
    build_line_table(fd, table, &line_count);
    
    // Print the table for debugging
    printf("\nLine Table:\n");
    printf("Line\tIndent\tLength\tOffset\n");
    printf("----\t------\t------\t------\n");
    print_line_table(table, line_count);
    
    // Interactive loop with timeout
    while (1) {
        line_num = get_line_number_with_timeout(line_count, fd);
        if (line_num == 0) {
            break;
        }
        
        if (line_num > 0 && line_num <= line_count) {
            read_and_print_line(fd, table, line_num - 1); // Convert to 0-based indexing
        } else {
            printf("Invalid line number. Please enter a number between 1 and %d\n", line_count);
        }
    }
    
    close(fd);
    printf("Program ended.\n");
    return 0;
}

void build_line_table(int fd, LineTable *table, int *line_count) {
    off_t current_offset = 0;
    int indent_level, line_length;
    char ch;
    int bytes_read;
    
    // Reset file pointer to beginning
    lseek(fd, 0L, SEEK_SET);
    
    *line_count = 0;
    indent_level = 0;
    line_length = 0;
    
    while ((bytes_read = read(fd, &ch, 1)) > 0) {
        if (ch == '\n') {
            // End of line
            table[*line_count].indent_level = indent_level;
            table[*line_count].line_length = line_length;
            table[*line_count].file_offset = current_offset - line_length;
            (*line_count)++;
            
            // Reset for next line
            indent_level = 0;
            line_length = 0;
        } else {
            if (ch == ' ') {
                indent_level++;
            } else if (ch == '\t') {
                indent_level += 4; // Assuming tab is 4 spaces
            }
            line_length++;
        }
        
        current_offset++;
    }
    
    // Handle case where file doesn't end with newline
    if (line_length > 0) {
        table[*line_count].indent_level = indent_level;
        table[*line_count].line_length = line_length;
        table[*line_count].file_offset = current_offset - line_length;
        (*line_count)++;
    }
}

void print_line_table(LineTable *table, int line_count) {
    for (int i = 0; i < line_count; i++) {
        printf("%d\t%d\t%d\t%ld\n", 
               i + 1, 
               table[i].indent_level, 
               table[i].line_length, 
               table[i].file_offset);
    }
}

void read_and_print_line(int fd, LineTable *table, int line_num) {
    char *buffer;
    off_t offset;
    int length;
    
    // Get the offset and length for the specified line
    offset = table[line_num].file_offset;
    length = table[line_num].line_length;
    
    // Allocate buffer for the line
    buffer = malloc(length + 1);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        return;
    }
    
    // Seek to the beginning of the line
    lseek(fd, offset, SEEK_SET);
    
    // Read the line
    if (read(fd, buffer, length) < 0) {
        perror("Error reading line");
        free(buffer);
        return;
    }
    
    // Null terminate and print
    buffer[length] = '\0';
    printf("Line %d: %s\n", line_num + 1, buffer);
    
    free(buffer);
}

int get_line_number_with_timeout(int max_lines, int fd) {
    int line_num;
    char input[32];
    char *endptr;
    int result;
    
    // Set up signal handling for alarm
    signal(SIGALRM, alarm_handler);
    
    while (1) {
        printf("\nEnter line number (1-%d) or 0 to quit: ", max_lines);
        fflush(stdout);
        
        if (!fl){
            alarm(TIMEOUT_SECONDS);
        }
        // Set alarm for 5 seconds

        timeout_occurred = 0;
        
        // Jump to this point if timeout occurs
        if (setjmp(jump_buffer) == 0) {
            // Normal execution path
            if (fgets(input, sizeof(input), stdin) == NULL) {
                alarm(0); // Cancel alarm
                return 0;
            }
            fl = 1;
            
            // Cancel alarm since input was received
            alarm(0);
            
            // Remove newline character
            input[strcspn(input, "\n")] = '\0';
            
            // Validate input
            if (strlen(input) == 0) {
                printf("Please enter a valid number.\n");
                continue;
            }
            
            errno = 0;
            line_num = (int)strtol(input, &endptr, 10);
            
            // Check for conversion errors
            if (*endptr != '\0' || errno == ERANGE) {
                printf("Invalid input. Please enter a valid number.\n");
                continue;
            }
            
            // Check range
            if (line_num < 0 || line_num > max_lines) {
                printf("Number out of range. Please enter a number between 0 and %d\n", max_lines);
                continue;
            }
            
            return line_num;
        } else {
            // Timeout occurred - display entire file content
            printf("\nTimeout! No input received within %d seconds.\n", TIMEOUT_SECONDS);
            printf("Displaying entire file content:\n");
            printf("==============================\n");
            
            // Save current position
            off_t current_pos = lseek(fd, 0, SEEK_CUR);
            
            // Reset to beginning of file
            lseek(fd, 0, SEEK_SET);
            
            // Read and display entire file
            char buffer[1024];
            int bytes_read;
            while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
                write(STDOUT_FILENO, buffer, bytes_read);
            }
            
            printf("\n==============================\n");
            printf("Program terminating due to timeout.\n");
            exit(1);
        }
    }
}

void alarm_handler(int sig) {
    // Set flag and jump back to main loop
    timeout_occurred = 1;
    longjmp(jump_buffer, 1);
}

int get_char(void) {
    struct termios old_termios;
    char ch;
    
    tcgetattr(STDIN_FILENO, &old_termios);
    old_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
    
    ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
    return ch;
}

int kbhit(void) {
    int ch = getchar();
    int oldf;
    
    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    
    return 0;
}

void clear_screen(void) {
    printf("\033[2J\033[H");
}
