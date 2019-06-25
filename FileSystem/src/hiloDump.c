/*
 * hiloDump.c
 *
 *  Created on: 26 may. 2019
 *      Author: utnso
 */
#include "hiloDump.h"


extern structConfig * config;
extern t_log * alog;
extern t_dictionary *memtable;

void crearTemporal(char * key, st_tabla* data){
	sem_wait(&data->semaforo);
    char* nombreArchivo = buscarNombreProximoTemporal(key);
    char* str = list_fold(data->lista, string_new(), (void*)armarStrLista);

    t_list* bloques = crearArchivoTemporal(nombreArchivo, sizeof(str));
    structParticion * dataBlq = leerParticion(nombreArchivo);


    if(bloques->elements_count > 0){
        //ITERAR POR EL TAMANIO Y PONER DATA
        /*char* path = armar_PathBloque(string_itoa(nbloque));
        FILE *write_ptr;

        write_ptr = fopen(path,"wb");

        fwrite(str,sizeof(str),string_length(str),write_ptr);
        fclose(write_ptr);

        free(path);*/
    }


    free(str);
	free(nombreArchivo);
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
    while (file = fopen(filename, "r")){
        fclose(file);
        i++;
        free(filename);
        filename = string_from_format("%s/%d.tmp", pathTabla, i);
    }
    free(pathTabla);
    return filename;
}

void* hilodump(){

	//signal(SIGKILL,senial);
	while(1){
		sleep(config->tiempo_dump);
		dictionary_iterator(memtable,(void*)crearTemporal);
	}

	pthread_exit(NULL);
}


