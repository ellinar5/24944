#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <ctype.h>

#define BACKSCAPE 127
#define CTRL_U 21
#define CTRL_W 23
#define CTRL_D 4
#define CTRL_G 7         // звуковой сигнал
#define MAX_LINE_LEN 40  // максимальная длина строки

void disable_echo_and_canonical(struct termios *old_tio)
{
    struct termios new_tio;
    tcgetattr(STDIN_FILENO, old_tio);
    new_tio = *old_tio;
    new_tio.c_lflag &= ~ICANON;
    new_tio.c_lflag &= ~ECHO;
    new_tio.c_cc[VMIN] = 1;
    new_tio.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

// перерисовка строки с учетом позиции курсора
void redraw_line(const char *line, int pos)
{
    write(STDOUT_FILENO, "\r\033[K", 4);
    write(STDOUT_FILENO, line, strlen(line));
    
    // возврат курсора на позицию
    int move_back = strlen(line) - pos;
    if (move_back > 0)
    {
        char move_seq[16];
        snprintf(move_seq, sizeof(move_seq), "\033[%dD", move_back);
        write(STDOUT_FILENO, move_seq, strlen(move_seq));
    }
}

int main()
{
    struct termios old_tio;
    disable_echo_and_canonical(&old_tio);

    char line[MAX_LINE_LEN + 1] = {0};
    int pos = 0;
    int column = 0;
    char beep = CTRL_G;

    printf("Введите текст: ");
    fflush(stdout);

    while (1)
    {
        char c;
        read(STDIN_FILENO, &c, 1);

        if (c == CTRL_D)
        { 
            if (pos == 0 && column == 0) { break; } 
        }
        else if (c == BACKSCAPE)
        { 
            if (pos > 0)
            {
                memmove(&line[pos - 1], &line[pos], strlen(line) - pos + 1);
                pos--;
                column--;
                redraw_line(line, pos);
            }
            else { write(STDOUT_FILENO, &beep, 1); } 
        }
        else if (c == CTRL_U)
        { 
            if (pos > 0)
            {
                memset(line, 0, MAX_LINE_LEN + 1);
                column = 0;
                pos = 0;
                redraw_line(line, pos);
            }
            else { write(STDOUT_FILENO, &beep, 1); }
        }
        else if (c == CTRL_W)
        { 
            if (pos == 0) { continue; }
    
            int search_position = pos - 1;
            
            // Удаляем пробелы в конце
            while (search_position >= 0 && line[search_position] == ' ') { search_position--; }
            
            // Удаляем последнее слово
            while (search_position >= 0 && line[search_position] != ' ') { search_position--; }
            
            search_position++; // переходим к первому символу слова
            
            // Вычисляем сколько символов нужно удалить
            int chars_to_delete = pos - search_position;
            
            // Сдвигаем оставшуюся часть строки и обнуляем хвост
            memmove(&line[search_position], &line[pos], strlen(line) - pos + 1);
            
            // Очищаем освободившуюся память в конце строки
            memset(&line[strlen(line)], 0, chars_to_delete);
            
            pos = search_position;
            column = search_position;
            
            redraw_line(line, pos); 
        }
        else if (c >= 32 && c <= 126) // печатаемые символы
        {
            if (strlen(line) < MAX_LINE_LEN)
            {
                // вставка символа в текущую позицию
                if (pos < strlen(line)) { 
                    memmove(&line[pos + 1], &line[pos], strlen(line) - pos + 1); 
                }
                line[pos] = c;
                pos++;
                column++;

                // Проверка на необходимость переноса
                if (column >= MAX_LINE_LEN) 
                {
                    int word_start = pos - 1;
                    
                    // Ищем начало текущего слова (ищем пробел перед словом)
                    while (word_start > 0 && line[word_start] != ' ') {
                        word_start--;
                    }
                    
                    // Если нашли пробел, то word_start указывает на пробел
                    // Начало слова - следующая позиция после пробела
                    if (line[word_start] == ' ') {
                        word_start++;
                    }
                    
                    // Если слово начинается с начала строки и слишком длинное
                    if (word_start == 0 && pos >= MAX_LINE_LEN) 
                    {
                        write(STDOUT_FILENO, &beep, 1);
                        // Откатываем последний символ
                        memmove(&line[pos - 1], &line[pos], strlen(line) - pos + 1);
                        pos--;
                        column--;
                        continue;
                    }

                    // Выделяем слово для переноса
                    char word_to_wrap[MAX_LINE_LEN] = {0};
                    int word_len = pos - word_start;
                    strncpy(word_to_wrap, &line[word_start], word_len);
                    
                    // Обрезаем текущую строку ДО начала переносимого слова
                    line[word_start] = '\0';
                    
                    // Перерисовываем текущую строку (без перенесенного слова)
                    pos = word_start;
                    column = word_start;
                    redraw_line(line, pos);

                    memset(line, 0, sizeof(line));
                    
                    // Переходим на новую строку и выводим перенесенное слово
                    write(STDOUT_FILENO, "\n", 1);
                    write(STDOUT_FILENO, word_to_wrap, word_len);
                    
                    // Копируем перенесенное слово как новую текущую строку
                    strcpy(line, word_to_wrap);
                    pos = word_len;
                    column = word_len;
                }
                else { redraw_line(line, pos); }
            }
            else { write(STDOUT_FILENO, &beep, 1); }
        }
        else if (c == '\n' || c == '\r') // новая строка
        {
            write(STDOUT_FILENO, "\n", 1);
            // printf("Вы ввели: %s\n", line);
            // printf("Введите текст: ");
            fflush(stdout);
            memset(line, 0, sizeof(line));
            pos = 0;
            column = 0;
        }
        else { write(STDOUT_FILENO, &beep, 1); }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    write(STDOUT_FILENO, "\n", 1);
    return 0;
}