#include <stdio.h>
#include <unistd.h>
#include <termios.h>

int main() {
    struct termios oldt, newt;
    char ch;

    /* Сохраняем текущие настройки терминала */
    tcgetattr(STDIN_FILENO, &oldt);

    /* Копируем их */
    newt = oldt;

    /* Отключаем канонический режим и эхо */
    newt.c_lflag &= ~(ICANON | ECHO);

    /* Минимум 1 символ, без ожидания Enter */
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;

    /* Применяем новые настройки */
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    /* Вопрос */
    printf("Вы уверены? (y/n): ");
    fflush(stdout);

    /* Считываем один символ */
    read(STDIN_FILENO, &ch, 1);

    printf("\nВы ввели: %c\n", ch);

    /* Восстанавливаем старые настройки терминала */
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return 0;
}