/*
 ============================================================================
 Name        : 3.c
 Author      : Sam Lunev
 Version     : .0
 Copyright   : All rights reserved
 Description : OSLrCPT3
 ============================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

int main() {
    uid_t real_uid, effective_uid;
    FILE *fp;
    char filename[] = "testfile.txt";
    
    // Get real and effective user IDs
    real_uid = getuid();
    effective_uid = geteuid();
    
    printf("Real User ID: %d\n", real_uid);
    printf("Effective User ID: %d\n", effective_uid);
    
    // Check if they match
    if (real_uid == effective_uid) {
        printf("Real and Effective UIDs match\n");
    } else {
        printf("Real and Effective UIDs do NOT match\n");
    }
    
    // Create a data file with permissions 600 (read/write for owner only)
    fp = fopen(filename, "w");
    if (fp != NULL) {
        fprintf(fp, "This is a test file accessible only by the owner.\n");
        fclose(fp);
        printf("File '%s' created successfully\n", filename);
    } else {
        perror("fopen failed");
        return 1;
    }
    
    // Now try to open the file again
    fp = fopen(filename, "r");
    if (fp != NULL) {
        printf("Successfully opened file '%s' for reading\n", filename);
        fclose(fp);
    } else {
        perror("fopen failed on second attempt");
        return 1;
    }
    
    return 0;
}
