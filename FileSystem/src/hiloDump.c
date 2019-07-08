/*
 * hiloDump.c
 *
 *  Created on: 26 may. 2019
 *      Author: utnso
 */
#include "hiloDump.h"


extern structConfig * config;
extern t_dictionary *memtable;
extern int tBloques;
extern int loop;

void crearTemporal(char * key, st_tabla* data){
	sem_wait(&data->semaforo);
    char* nombreArchivo = buscarNombreProximoTemporal(key);
    char* str = list_fold(data->lista, string_new(), (void*)armarStrLista);

    t_list* bloques = crearArchivoTemporal(nombreArchivo, sizeof(str));


    int tamanioRestante = (int) sizeof(str), iElem = 0;

    if(bloques->elements_count > 0){
        //ITERAR POR EL TAMANIO Y PONER DATA
        int* numeroBloque = list_get(bloques, 0);
        int caracteresPorString = tBloques * 1024/ sizeof(char);
        while(tamanioRestante > 0 && numeroBloque != NULL){

            char* path = armar_PathBloque(string_itoa(*numeroBloque));
            FILE *write_ptr;

            write_ptr = fopen(path,"ab+");
            char* strBloque = string_substring(str, iElem * caracteresPorString, caracteresPorString);
            fwrite(strBloque,sizeof(strBloque),string_length(strBloque),write_ptr);
            fclose(write_ptr);

            free(path);
            free(strBloque);

            iElem++;
            free(numeroBloque);
            numeroBloque = list_get(bloques,iElem);
            tamanioRestante -= tBloques;

        }
    }


    free(str);
	free(nombreArchivo);
	list_destroy(bloques);
    sem_post(&data->semaforo);
    free(dictionary_remove(memtable,key));
}

char* armarStrLista(char * strLista, structRegistro *registro){
   string_append_with_format(&strLista, "%d;%d;%s\n", registro->time, registro->key, registro->value);
   return strLista;
}

char* buscarNombreProximoTemporal(char* nombreTabla){
    char* pathTabla = armar_path(nombreTabla);
    FILE *file;
    int i = 1;
    char* filename = string_from_format("%s/%d.tmp", pathTabla, i);
    while ((file = fopen(filename, "r"))){
        fclose(file);
        i++;
        free(filename);
        filename = string_from_format("%s/%d.tmp", pathTabla, i);
    }
    free(pathTabla);
    return filename;
}

void* hilodump(){

	while(loop){
		sleep(config->tiempo_dump);
		dictionary_iterator(memtable,(void*)crearTemporal);
	}

	pthread_exit(NULL);
}


