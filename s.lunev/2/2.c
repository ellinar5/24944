/*
 ============================================================================
 Name        : 2.c
 Author      : Sam Lunev
 Version     : .0
 Copyright   : All rights reserved
 Description : OS LrCrsPT2
 ============================================================================
 */


#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

extern char *tzname[];

int main()
{
    time_t now;
    struct tm *sp;
    char *old_tz;

    // Save current timezone
    old_tz = getenv("TZ");
    
    // Set timezone to Pacific Standard Time (PST)
    setenv("TZ", "PST8PDT", 1);
    
    // Initialize timezone information
    tzset();
    
    (void) time(&now);

    printf("%s", ctime(&now));

    sp = localtime(&now);
    printf("%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year + 1900, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst]);
    
    // Restore original timezone
    if (old_tz) {
        setenv("TZ", old_tz, 1);
    } else {
        unsetenv("TZ");
    }
    tzset();
    
    exit(0);
}
