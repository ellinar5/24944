#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

volatile bool timer_expired = false;

void handle_timer(int signal_num) { timer_expired = true; }

int main()
{
    int file_handler = open("input.txt", O_RDONLY);

    char data_buffer[1024];
    int bytes_processed = read(file_handler, data_buffer, sizeof(data_buffer));

    int total_lines = 0;
    for (int counter = 0; counter < bytes_processed; counter++) { 
        if (data_buffer[counter] == '\n') { total_lines++; } 
    }

    if (data_buffer[bytes_processed-1] != '\n') { total_lines++; }

    int line_offsets[total_lines + 1];
    line_offsets[0] = 0;
    int position_index = 1;

    for (int counter = 0; counter < bytes_processed; counter++)
    {
        if (data_buffer[counter] == '\n') 
        {
            line_offsets[position_index] = counter + 1;
            position_index++;
        }
    }

    line_offsets[total_lines] = bytes_processed;

    printf("== String number to position mapping ==\n");
    for (int line_num = 0; line_num < total_lines; line_num++)
    {
        lseek(file_handler, line_offsets[line_num], SEEK_SET);

        int line_size = line_offsets[line_num+1] - line_offsets[line_num];
        char line_content[line_size];
        read(file_handler, line_content, line_size);

        printf("%d\t%d\t%d", line_num+1, line_offsets[line_num], line_size);
        printf("\n");
    }

    signal(SIGALRM, handle_timer);
    int timer_active = 0;

    while(1)
    {
        printf("Enter line number (1-%d): ", total_lines);
        fflush(stdout);

        if (timer_active == 0)
        {
            timer_expired = false;
            alarm(5);
            timer_active = 1;
        }
        
        int user_input;
        int input_status = scanf("%d", &user_input);

        alarm(0);
        
        if (timer_expired) {
            printf("\nFull file content:\n");
            for (int i = 0; i < bytes_processed; i++) { 
                printf("%c", data_buffer[i]); 
            }
            printf("\n");
            break;
        }

        if (input_status != 1)
        {
            printf("Invalid input format\n");
            int cleanup_char;
            while ((cleanup_char = getchar()) != '\n' && cleanup_char != EOF);
            continue;
        }

        if (user_input == 0) { break; }
        else if (user_input > total_lines || user_input < 1) { 
            printf("Line number out of range\n"); 
            continue; 
        }

        if (input_status == 1)
        {
            lseek(file_handler, line_offsets[user_input-1], SEEK_SET);

            int selected_line_size = line_offsets[user_input] - line_offsets[user_input-1];
            char selected_line_content[selected_line_size];
            read(file_handler, selected_line_content, selected_line_size);

            for (int char_index = 0; char_index < selected_line_size; char_index++) { 
                printf("%c", selected_line_content[char_index]); 
            }
            printf("\n");
        }
    }

    close(file_handler);

    return 0;
}