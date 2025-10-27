#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

volatile bool timeout = false;

void timeout_handler(int sig) { timeout = true; }

int main()
{
    int st = open("test.txt", O_RDONLY);
    struct stat file_info;
    fstat(st, &file_info); //получение информации о файле

    int file_size = file_info.st_size; //размер в байтах
    char* text = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, st, 0);

    int count_str = 0;
    for (int i = 0; i < file_size; i++) { if (text[i] == '\n') { count_str++;} }

    if (text[file_size - 1] != '\n') { count_str++; }

    int pos_str[count_str + 1];
    pos_str[0] = 0;
    int idx = 1;

    for (int i = 0; i < file_size; i++)
    {
        if (text[i] == '\n') 
        {
            pos_str[idx] = i + 1;
            idx++;
        }
    }

    for (int i = 0; i < file_size; i++)
    {
        if (text[i] == '\n') 
        {
            pos_str[idx] = i + 1;
            idx++;
        }
    }

    pos_str[count_str] = file_size;

    printf("== Таблица соответствия строки и номера ==\n");
    for (int j = 0; j < count_str; j++)
    {
        printf("%d	%d	%d", j+1, pos_str[j], pos_str[j+1] - pos_str[j]);
        printf("\n");
    }

    signal(SIGALRM, timeout_handler);
    int flag = 0;

    while(1)
    {
        printf("Введите номер строки (1-%d): ", count_str);
	fflush(stdout);

	if (flag == 0)
	{
		timeout = false;
        	alarm(5);
		flag =1;
	}	

        int n;
        int result = scanf("%d", &n);

	alarm(0);
	
	if (timeout) {
            printf("\nВывод всего файла:\n");
            for (int i = 0; i < file_size; i++) { printf("%c", text[i]); }
	    printf("\n");
            break;
        }
	
	

	if (result != 1)
	{
		printf("Invalid index2\n");
		int c;
        	while ((c = getchar()) != '\n' && c != EOF);
		continue;
	}

        if (n == 0) { break; }
	else if (n > count_str || n < 1) { printf("Invalid index\n"); continue; 
		
	}

        if (result == 1)
        {
            for (int i = pos_str[n-1]; i < pos_str[n]; i++) { printf("%c", text[i]); }
	    printf("\n");
        }
    }

    munmap(text, file_info.st_size);
    close(st);

    return 0;
}
