/*
 * hiloDump.c
 *
 *  Created on: 26 may. 2019
 *      Author: utnso
 */
#include "hiloDump.h"
#include "hiloInotify.h"
#include "Semaforos.h"

extern t_dictionary *memtable;
extern int tBloques;
extern int loop;
extern sem_t sMemtable;
extern t_log* alog;

void crearTemporal(char * key, st_tabla* data){
	sem_wait(&data->semaforo);
    char* nombreArchivo = buscarNombreProximoTemporal(key);
    char* str = list_fold(data->lista, strdup(""), (void*)armarStrLista);

    t_list* bloques = crearArchivoTemporal(nombreArchivo, string_length(str) * 4);


    int tamanioRestante = string_length(str) * 4, iElem = 0;

    if(bloques->elements_count > 0){
        //ITERAR POR EL TAMANIO Y PONER DATA
        int* numeroBloque = list_get(bloques, 0);
        int caracteresPorString = tBloques / (4 * sizeof(char));
        while(tamanioRestante > 0 && numeroBloque != NULL){

        	char * bloque = string_itoa(*numeroBloque);
            char* path = armar_PathBloque(bloque);
            free(bloque);
            FILE *write_ptr;

            remove(path);
            write_ptr = fopen(path,"w");
            char* strBloque = string_substring(str, iElem * caracteresPorString, caracteresPorString);
            //write(1, strBloque, strlen(strBloque)+1);
            //fwrite(strBloque,sizeof(strBloque),string_length(strBloque),write_ptr);
            fputs(strBloque, write_ptr);
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
}

char* armarStrLista(char * strLista, structRegistro *registro){
   string_append_with_format(&strLista, "%.0f;%d;%s\n", registro->time, registro->key, registro->value);
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
		sleep(getDump());
		sem_wait(&sMemtable);
		log_info(alog,"Empieza el dump");
		dictionary_iterator(memtable,(void*)crearTemporal);
		dictionary_clean_and_destroy_elements(memtable, (void*)limpiarMemtable);
		log_info(alog, "Termina el dump");
		sem_post(&sMemtable);

	}

	pthread_exit(NULL);
}

void limpiarMemtable(st_tabla * tabla){
	sem_destroy(&tabla->semaforo);
	list_destroy_and_destroy_elements(tabla->lista, (void*)limpiarLista);
	free(tabla);
}

void limpiarLista(structRegistro * reg){
	free(reg->value);
	free(reg);
}


