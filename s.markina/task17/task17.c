#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <ctype.h>

// Управляющие символы
#define ERASE_CHAR 127    // Backspace
#define LINE_KILL 21      // Ctrl+U - удаление всей строки
#define WORD_ERASE 23     // Ctrl+W - удаление слова
#define EXIT_PROGRAM 4    // Ctrl+D - выход из программы
#define BEEP_SIGNAL 7     // Ctrl+G - звуковой сигнал
#define MAX_INPUT_WIDTH 40 // Максимальная длина строки

// Отключение стандартного режима терминала
void setup_raw_terminal(struct termios *original_settings) {
    struct termios new_settings;
    // Получаем текущие настройки терминала
    tcgetattr(STDIN_FILENO, original_settings);
    new_settings = *original_settings;
    
    new_settings.c_lflag &= ~(ICANON | ECHO);
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    
    // Применяем новые настройки
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
}

// Обновление текущей строки с позиционированием курсора
void refresh_display(const char *input_line, int cursor_position) {
    write(STDOUT_FILENO, "\r\033[K", 4);
    write(STDOUT_FILENO, input_line, strlen(input_line));
    
    // Возврат курсора на нужную позицию
    int backward_steps = strlen(input_line) - cursor_position;
    if (backward_steps > 0) {
        char cursor_command[16];
        snprintf(cursor_command, sizeof(cursor_command), "\033[%dD", backward_steps);
        write(STDOUT_FILENO, cursor_command, strlen(cursor_command));
    }
}

// Удаление последнего слова в строке
void remove_previous_word(char *text, int *current_position, int *column_position) {
    if (*current_position == 0) return;
    int search_index = *current_position - 1;
    
    // Пропускаем пробелы перед словом и находим начало
    while (search_index >= 0 && text[search_index] == ' ') {
        search_index--;
    }
    while (search_index >= 0 && text[search_index] != ' ') {
        search_index--;
    }
    search_index++;
    
    int deletion_count = *current_position - search_index;  // Количество удаляемых символов
    
    // Удаляем слово
    memmove(&text[search_index], &text[*current_position], 
            strlen(text) - *current_position + 1);
    
    *current_position = search_index;
    *column_position = search_index;
    refresh_display(text, *current_position);
}

// Обработка переноса слова при достижении границы
void handle_word_wrap(char *text, int *current_pos, int *col_pos) {
    int text_length = strlen(text);
    if (*col_pos < MAX_INPUT_WIDTH) return;
    
    // Ищем начало текущего слова для переноса
    int word_begin = *current_pos - 1;
    while (word_begin > 0 && text[word_begin] != ' ') {
        word_begin--;
    }
    
    if (text[word_begin] == ' ') {
        word_begin++;
    }
    
    // Если слово слишком длинное и не помещается
    if (word_begin == 0 && *current_pos >= MAX_INPUT_WIDTH) {
        char beep_char = BEEP_SIGNAL;  // Создаем переменную для звукового сигнала
        write(STDOUT_FILENO, &beep_char, 1);
        memmove(&text[*current_pos - 1], &text[*current_pos], 
                text_length - *current_pos + 1);
        (*current_pos)--;
        (*col_pos)--;
        return;
    }
    
    int word_length = *current_pos - word_begin;
    char wrapped_word[MAX_INPUT_WIDTH] = {0};
    strncpy(wrapped_word, &text[word_begin], word_length);
    text[word_begin] = '\0';
    
    *current_pos = word_begin;
    *col_pos = word_begin;
    refresh_display(text, *current_pos);
    
    write(STDOUT_FILENO, "\n", 1);
    write(STDOUT_FILENO, wrapped_word, word_length);
    
    strcpy(text, wrapped_word);
    *current_pos = word_length;
    *col_pos = word_length;
}

int main() {
    struct termios original_terminal;
    setup_raw_terminal(&original_terminal);

    char input_buffer[MAX_INPUT_WIDTH + 1] = {0};
    int cursor_pos = 0;
    int screen_column = 0;
    char beep_char = BEEP_SIGNAL;  // Переменная для звукового сигнала

    printf("Введите текст: ");
    fflush(stdout);

    while (1) {
        char input_char;
        read(STDIN_FILENO, &input_char, 1);

        // Выход (Ctrl+D)
        if (input_char == EXIT_PROGRAM) { 
            if (cursor_pos == 0 && screen_column == 0) {
                break; 
            }
        }
        // Backspace
        else if (input_char == ERASE_CHAR) { 
            if (cursor_pos > 0) {
                memmove(&input_buffer[cursor_pos - 1], &input_buffer[cursor_pos], 
                        strlen(input_buffer) - cursor_pos + 1);
                cursor_pos--;
                screen_column--;
                refresh_display(input_buffer, cursor_pos);
            } else {
                write(STDOUT_FILENO, &beep_char, 1); // Сигнал при невозможности удаления
            }
        }
        // Удаление всей строки (Ctrl+U)
        else if (input_char == LINE_KILL) { 
            if (cursor_pos > 0) {
                memset(input_buffer, 0, MAX_INPUT_WIDTH + 1);
                screen_column = 0;
                cursor_pos = 0;
                refresh_display(input_buffer, cursor_pos);
            } else {
                write(STDOUT_FILENO, &beep_char, 1);
            }
        }
        // Удаление слова (Ctrl+W)
        else if (input_char == WORD_ERASE) { 
            if (cursor_pos == 0) continue;
            remove_previous_word(input_buffer, &cursor_pos, &screen_column);
        }
        // Печатаемые символы
        else if (input_char >= 32 && input_char <= 126) {
            if (strlen(input_buffer) < MAX_INPUT_WIDTH) {
                if (cursor_pos < strlen(input_buffer)) { 
                    memmove(&input_buffer[cursor_pos + 1], &input_buffer[cursor_pos], 
                            strlen(input_buffer) - cursor_pos + 1); 
                }
                input_buffer[cursor_pos] = input_char;
                cursor_pos++;
                screen_column++;
                if (screen_column >= MAX_INPUT_WIDTH) {
                    handle_word_wrap(input_buffer, &cursor_pos, &screen_column);
                } else {
                    refresh_display(input_buffer, cursor_pos);
                }
            } else {
                write(STDOUT_FILENO, &beep_char, 1);
            }
        }
        // Enter
        else if (input_char == '\n' || input_char == '\r') {
            write(STDOUT_FILENO, "\n", 1);
            memset(input_buffer, 0, sizeof(input_buffer));
            cursor_pos = 0;
            screen_column = 0;
        }
        // Недопустимые символы
        else {
            write(STDOUT_FILENO, &beep_char, 1);
        }
    }

    // Восстановление оригинальных настроек терминала
    tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal);
    write(STDOUT_FILENO, "\n", 1);
    return 0;
}