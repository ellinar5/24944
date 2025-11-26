#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>

static struct termios orig_termios;

static void restore_terminal(void) {
    /* Восстановление оригинальных атрибутов терминала */
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

/* Обработчик сигналов — восстанавливаем терминал и завершаем программу */
static void handle_signal(int sig) {
    restore_terminal();
    _exit(128 + sig);
}

int main(void) {
    struct termios new_termios;
    unsigned char ch;

    /* Сохраняем текущие настройки терминала */
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        return 1;
    }

    /* Регистрируем восстановление при нормальном завершении */
    if (atexit(restore_terminal) != 0) {
        fprintf(stderr, "atexit registration failed\n");
        return 1;
    }

    /* Настроим обработчики сигналов, чтобы в случае прерывания терминал тоже восстановился */
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGQUIT, handle_signal);

    /* Скопируем оригинальные настройки и модифицируем */
    new_termios = orig_termios;
    /* Выключаем канонический режим и эхо:
       - ICANON: ввод буферизуется до Enter (выключаем — читаем по байту)
       - ECHO: не показывать вводимый символ (можно убрать, если нужно видеть символ) */
    new_termios.c_lflag &= ~(ICANON | ECHO);
    /* Минимальное количество символов для чтения = 1, таймаут = 0 */
    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_termios) == -1) {
        perror("tcsetattr");
        return 1;
    }

    /* Печатаем вопрос и принудительно сбрасываем буфер вывода */
    printf("Continue? (y/n): ");
    fflush(stdout);

    /* Читаем ровно один байт — пользователь не должен нажимать Enter */
    ssize_t r = read(STDIN_FILENO, &ch, 1);
    if (r <= 0) {
        perror("read");
        return 1;
    }

    /* Печатаем результат и перевод строки для аккуратного вывода */
    printf("%c\n", ch);

    /* Вопрос можно обработать как нужно */
    if (ch == 'y' || ch == 'Y') {
        printf("You chose YES\n");
    } else if (ch == 'n' || ch == 'N') {
        printf("You chose NO\n");
    } else {
        printf("You pressed: 0x%02x\n", ch);
    }

    /* restore_terminal будет вызвана автоматически через atexit */
    return 0;
}
