/*
 ============================================================================
 Name        : 1.c
 Author      : Sam Lunev
 Version     : .0
 Copyright   : All rights reserved
 Description : Operation Systems Learning Course Practicum Task 1.
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

void print_ids(void);
void set_group_leader(void);
void print_pids(void);
void print_ulimit(void);
void change_ulimit(const char *new_ulimit);
void print_core_size(void);
void set_core_size(const char *size);
void print_working_dir(void);
void print_env_vars(void);
void add_env_var(const char *name_value);
int validate_number(const char *str);
int is_valid_env_name(const char *name);

int main(int argc, char *argv[]) {
    int opt;
    int i_flag = 0;
    int s_flag = 0;
    int p_flag = 0;
    int u_flag = 0;
    int U_flag = 0;
    int c_flag = 0;
    int C_flag = 0;
    int d_flag = 0;
    int v_flag = 0;
    int V_flag = 0;
    
    char *U_value = NULL;
    char *C_value = NULL;
    char *V_value = NULL;
    
    // Process options from right to left
    while ((opt = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        switch (opt) {
            case 'i':
                i_flag = 1;
                break;
            case 's':
                s_flag = 1;
                break;
            case 'p':
                p_flag = 1;
                break;
            case 'u':
                u_flag = 1;
                break;
            case 'U':
                U_flag = 1;
                U_value = optarg;
                break;
            case 'c':
                c_flag = 1;
                break;
            case 'C':
                C_flag = 1;
                C_value = optarg;
                break;
            case 'd':
                d_flag = 1;
                break;
            case 'v':
                v_flag = 1;
                break;
            case 'V':
                V_flag = 1;
                V_value = optarg;
                break;
            case '?':
                // getopt already prints error message
                fprintf(stderr, "Usage: %s [options]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    
    // Validate U option value
    if (U_flag && U_value) {
        if (!validate_number(U_value)) {
            fprintf(stderr, "Error: Invalid ulimit value '%s'\n", U_value);
            exit(EXIT_FAILURE);
        }
    }
    
    // Validate C option value
    if (C_flag && C_value) {
        if (!validate_number(C_value)) {
            fprintf(stderr, "Error: Invalid core size value '%s'\n", C_value);
            exit(EXIT_FAILURE);
        }
    }
    
    // Validate V option value
    if (V_flag && V_value) {
        char *equal_pos = strchr(V_value, '=');
        if (equal_pos == NULL) {
            fprintf(stderr, "Error: Invalid environment variable format '%s'\n", V_value);
            exit(EXIT_FAILURE);
        }
        char *name = strndup(V_value, equal_pos - V_value);
        if (!is_valid_env_name(name)) {
            fprintf(stderr, "Error: Invalid environment variable name '%s'\n", name);
            free(name);
            exit(EXIT_FAILURE);
        }
        free(name);
    }
    
    // Process flags in order of appearance (right to left)
    if (V_flag) {
        add_env_var(V_value);
    }
    
    if (d_flag) {
        print_working_dir();
    }
    
    if (v_flag) {
        print_env_vars();
    }
    
    if (C_flag) {
        set_core_size(C_value);
    }
    
    if (c_flag) {
        print_core_size();
    }
    
    if (U_flag) {
        change_ulimit(U_value);
    }
    
    if (u_flag) {
        print_ulimit();
    }
    
    if (p_flag) {
        print_pids();
    }
    
    if (s_flag) {
        set_group_leader();
    }
    
    if (i_flag) {
        print_ids();
    }
    
    return 0;
}

void print_ids(void) {
    uid_t real_uid, effective_uid;
    gid_t real_gid, effective_gid;
    
    real_uid = getuid();
    effective_uid = geteuid();
    real_gid = getgid();
    effective_gid = getegid();
    
    printf("Real UID: %d\n", real_uid);
    printf("Effective UID: %d\n", effective_uid);
    printf("Real GID: %d\n", real_gid);
    printf("Effective GID: %d\n", effective_gid);
}

void set_group_leader(void) {
    pid_t pid = getpid();
    
    if (setpgid(pid, pid) == -1) {
        perror("setpgid");
        exit(EXIT_FAILURE);
    }
    
    printf("Process %d is now group leader\n", pid);
}

void print_pids(void) {
    pid_t pid = getpid();
    pid_t ppid = getppid();
    pid_t pgid = getpgrp();
    
    printf("Process ID: %d\n", pid);
    printf("Parent Process ID: %d\n", ppid);
    printf("Process Group ID: %d\n", pgid);
}

void print_ulimit(void) {
    struct rlimit rl;
    
    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("getrlimit");
        return;
    }
    
    if (rl.rlim_cur == RLIM_INFINITY) {
        printf("Core file size: unlimited\n");
    } else {
        printf("Core file size: %lu bytes\n", rl.rlim_cur);
    }
}

void change_ulimit(const char *new_ulimit) {
    struct rlimit rl;
    long limit;
    char *endptr;
    
    errno = 0;
    limit = strtol(new_ulimit, &endptr, 10);
    
    if (errno != 0 || *endptr != '\0') {
        fprintf(stderr, "Error: Invalid ulimit value '%s'\n", new_ulimit);
        exit(EXIT_FAILURE);
    }
    
    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("getrlimit");
        return;
    }
    
    if (limit == 0) {
        rl.rlim_cur = 0;
    } else if (limit == -1) {
        rl.rlim_cur = RLIM_INFINITY;
    } else if (limit < 0) {
        fprintf(stderr, "Error: Ulimit value must be non-negative\n");
        exit(EXIT_FAILURE);
    } else {
        rl.rlim_cur = limit;
    }
    
    if (setrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("setrlimit");
        exit(EXIT_FAILURE);
    }
    
    printf("Ulimit changed to: %ld bytes\n", limit);
}

void print_core_size(void) {
    struct rlimit rl;
    
    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("getrlimit");
        return;
    }
    
    if (rl.rlim_cur == RLIM_INFINITY) {
        printf("Core file size limit: unlimited\n");
    } else {
        printf("Core file size limit: %lu bytes\n", rl.rlim_cur);
    }
}

void set_core_size(const char *size_str) {
    struct rlimit rl;
    long size;
    char *endptr;
    
    errno = 0;
    size = strtol(size_str, &endptr, 10);
    
    if (errno != 0 || *endptr != '\0') {
        fprintf(stderr, "Error: Invalid core size value '%s'\n", size_str);
        exit(EXIT_FAILURE);
    }
    
    if (size < 0) {
        fprintf(stderr, "Error: Core size value must be non-negative\n");
        exit(EXIT_FAILURE);
    }
    
    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("getrlimit");
        exit(EXIT_FAILURE);
    }
    
    if (size == 0) {
        rl.rlim_cur = 0;
    } else if (size == -1) {
        rl.rlim_cur = RLIM_INFINITY;
    } else {
        rl.rlim_cur = size;
    }
    
    if (setrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("setrlimit");
        exit(EXIT_FAILURE);
    }
    
    printf("Core size limit set to: %ld bytes\n", size);
}

void print_working_dir(void) {
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
    printf("Current directory: %s\n", buffer);
}

void print_env_vars(void) {
    extern char **environ;
    char **env = environ;
    
    while (*env != NULL) {
        printf("%s\n", *env);
        env++;
    }
}

void add_env_var(const char *name_value) {        
    // Create a copy of the string to modify
    char *temp = strdup(name_value);
    if (temp == NULL) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }
        
    if (!is_valid_env_name(temp)) {
        fprintf(stderr, "Error: Invalid environment variable name '%s'\n", temp);
        free(temp);
        exit(EXIT_FAILURE);
    }
    
    if (putenv(temp) == -1) {
        perror("setenv");
        free(temp);
        exit(EXIT_FAILURE);
    }
    
    printf("Added/modified environment variable: %s\n", temp);
    free(temp);
}

int validate_number(const char *str) {
    if (str == NULL || *str == '\0') {
        return 0;
    }
    
    // Check for negative sign
    const char *p = str;
    if (*p == '-' || *p == '+') {
        p++;
        if (*p == '\0') {
            return 0; // Just a sign, not a number
        }
    }
    
    // Check that all remaining characters are digits
    while (*p != '\0') {
        if (!isdigit(*p)) {
            return 0;
        }
        p++;
    }
    
    // Convert to check range (optional but good practice)
    char *endptr;
    errno = 0;
    long val = strtol(str, &endptr, 10);
    
    if (errno == ERANGE) {
        return 0; // Number out of range
    }
    
    return 1;
}

int is_valid_env_name(const char *name) {
    if (name == NULL || *name == '\0') {
        return 0;
    }
    
    // First character must be letter or underscore
    if (!isalpha(*name) && *name != '_') {
        return 0;
    }
    
    // Check that all characters are valid (letters, digits, underscores)
    const char *p = name;
    while (*p != '\0') {
        if (!isalnum(*p) && *p != '_' && *p != '=') {
            return 0;
        }
        p++;
    }
    
    return 1;
}
