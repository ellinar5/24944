#include <stdio.h>
#include <errno.h>
#include <unistd.h>


void print_uidf()
{
    printf("Real: %d\n", getuid());
    printf("Effective: %d\n", geteuid());
}

int main()
{
    print_uidf();

    FILE* st = fopen("secret_file", "r");
    
    if (st == NULL) { perror("File opening error"); }
    else { printf("Succes\n"); }
    fclose(st);

    if (setuid(getuid()) == -1) { perror("setuid failed"); }
    print_uidf();

    FILE* fn = fopen("secret_file", "r");

    if (fn == NULL) { perror("File opening error"); }
    else { printf("Succes\n"); }
    fclose(fn);

    return 0;
}