//
// Created by miguelchauca on 16/07/19.
//
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <string.h>
int main(){
    FILE * fileScript = fopen("/home/miguelchauca/Documents/Operativo/workSpace/tp-2019-1c-misc/example/server.lql","r");
    char * line;
    size_t len = 0;
    if(fileScript == NULL){
        printf("no se puedo abrir el archivo\n");
        return -1;
    }
    while (getline(&line, &len, fileScript) != -1) {
        if(string_contains(line,"\n")){
            line = string_substring(line,0,string_length(line) - 1);
            printf("%s",line);
        }else{
            printf("%s",line);
        }

    }

    free(line);
    fclose(fileScript);

    return 0;
}
