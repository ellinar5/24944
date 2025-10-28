#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main()
{
    putenv("TZ=PST8PDT");
    
    time_t n;
    time(&n);
    
    printf("%s", ctime(&n));
    
    return 0;
}
