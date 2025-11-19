#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

static struct termios orig_term;

void again_terminal(void){
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
}

void setup_terminal(void){
    struct termios new_terminal;

    if (tcgetattr(STDIN_FILENO, &orig_term) == -1){
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    atexit(again_terminal);

    new_terminal = orig_term;
    new_terminal.c_lflag &= ~(ICANON | ECHO);
    new_terminal.c_cc[VMIN] = 1;
    new_terminal.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal) == -1){
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

void erase_last_char(char *line, int *pos){
    if (*pos > 0){
        (*pos)--;
        printf("\b \b");
        fflush(stdout);
        line[*pos] = '\0';
    }
    else{
        printf("%c", '\a');
        fflush(stdout);
    }
}

void kill(char *line, int *pos){
    while (*pos > 0){
        (*pos)--;
        printf("\b \b");
        fflush(stdout);
    }
    line[0] = '\0';
}

int is_space(char c){
    return (c == ' ' || c == '\t');
}

void erase_last_word(char *line, int *pos){
    int end_pos = *pos;
    while (*pos > 0 && is_space(line[*pos - 1])){
        (*pos)--;
        printf("\b \b");
        fflush(stdout);
    }

    while (*pos > 0 && !is_space(line[*pos - 1])){
        (*pos)--;
        printf("\b \b");
        fflush(stdout);
    }

    line[*pos] = '\0';

    if (end_pos == *pos){
        printf("%c", '\a');
        fflush(stdout);
    }
}

int is_printable(char c){
    return (c >= 32 && c <= 126);
}

int find_wrap_position(const char *line, int current_pos){
    if (current_pos <= 40)
        return -1;
    for (int i = 40 - 1; i >= 0; i--){
        if (is_space(line[i])){
            return i + 1;
        }
    }

    return 40;
}

void perform_word_wrap(char *line, int *pos, int *column){
    int wrap_pos = find_wrap_position(line, *pos);
    if (wrap_pos != -1 && wrap_pos < *pos){
        for (int i = 0; i < *column; i++){
            printf("\b \b");
        }
        for (int i = 0; i < wrap_pos; i++){
            printf("%c", line[i]);
        }
        printf("\n");
        for (int i = wrap_pos; i < *pos; i++){
            printf("%c", line[i]);
        }
        int remaining_chars = *pos - wrap_pos;
        for (int i = 0; i < remaining_chars; i++){
            line[i] = line[wrap_pos + i];
        }
        line[remaining_chars] = '\0';
        *pos = remaining_chars;
        *column = remaining_chars;
        fflush(stdout);
    }
}

void check_line_wrap(char *line, int *pos, int *column){
    if (*column >= 40){
        perform_word_wrap(line, pos, column);
    }
}

int main(void){
    char line[40 * 2] = {0};
    int pos = 0;
    int column = 0;
    char c;

    setup_terminal();

    printf("Текстовый редактор (максимум %d символов в строке с переносом)\n", 40);
    printf("Управление:\n");
    printf("  Backspace или Ctrl+H - удалить символ\n");
    printf("  Ctrl+U - очистить всю строку\n");
    printf("  Ctrl+W - удалить предыдущее слово\n");
    printf("  Ctrl+D в начале строки - выход\n");
    printf("  Другие управляющие символы - звуковой сигнал\n");
    printf("**********************************************\n");

    while (1){
        if (read(STDIN_FILENO, &c, 1) != 1){
            break;
        }
        if (c == 0x7F || c == 0x08){
            erase_last_char(line, &pos);
            if (column > 0){
                column--;
            }
        }
        else if (c == 0x15){
            kill(line, &pos);
            column = 0;
        }
        else if (c == 0x17){
            erase_last_word(line, &pos);
            column = pos;
        }
        else if (c == 0x04){
            if (pos == 0){
                printf("\nВыход\n");
                break;
            }
            else{
                printf("%c", '\a');
                fflush(stdout);
            }
        }
        else if (c == '\n' || c == '\r'){
            printf("\n");
            pos = 0;
            column = 0;
            line[0] = '\0';
        }
        else if (is_printable(c)){
            if (pos < 40 * 2 - 1){
                line[pos++] = c;
                line[pos] = '\0';
                printf("%c", c);
                fflush(stdout);
                column++;
                check_line_wrap(line, &pos, &column);
            }
            else{
                printf("%c", '\a');
                fflush(stdout);
            }
        }
        else{
            printf("%c", '\a');
            fflush(stdout);
        }
    }
    return 0;
}