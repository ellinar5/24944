#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

struct Spisok{
    char *str;
    struct Spisok *next;
};

int main(){
    char str[1024];
    struct Spisok *nachalo = NULL;
    struct Spisok *konec = NULL;
    while(1){
        if(fgets(str, 1024, stdin) == NULL){
            break;
        }

        if(str[0] == '.'){
            break;
        }

        if(strlen(str) > 0 && str[strlen(str) - 1] == '\n'){
            str[strlen(str) - 1] = '\0';
        }        
        char *new_str = (char*)malloc(strlen(str));
        if (new_str == NULL) {
            perror("Ошибка выделения памяти");
            exit(1);
        }
        strcpy(new_str, str);
        struct Spisok *new_yzel = (struct Spisok*)malloc(sizeof(struct Spisok));
        if(new_yzel == NULL){
            perror("Ошибка выделения памяти");
            exit(1);
        }
        
        new_yzel->str = new_str;
        new_yzel->next = NULL;
        if(nachalo == NULL){
            nachalo = new_yzel;
            konec = new_yzel;
        }
        else{
            konec->next = new_yzel;
            konec = new_yzel;
        }
    }
    struct Spisok *now_yzel = nachalo;
    printf("Все строки:\n");
    while(1){
        if(now_yzel->next == NULL){
            printf("%s\n",now_yzel->str);
            break;
        }
        else{
            printf("%s\n",now_yzel->str);
            now_yzel = now_yzel->next;
        }
    }

    now_yzel = nachalo;
    while(1){
        if(now_yzel->next == NULL){
            free(now_yzel->str);
            free(now_yzel);
            break;
        }
        else{
            struct Spisok *next = now_yzel->next;
            free(now_yzel->str);
            free(now_yzel);
            now_yzel = next;
        }
    }
    return 0;
}