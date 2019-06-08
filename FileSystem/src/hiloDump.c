/*
 * hiloDump.c
 *
 *  Created on: 26 may. 2019
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <commons/collections/dictionary.h>
#include "Funciones.h"
#include "manejoArchivos.h"
#include <semaphore.h>


extern structConfig * config;
extern t_dictionary *memtable;

void crearTemporal(char * key, st_tabla* data){
	sem_wait(&data->semaforo);


	sem_post(&data->semaforo);

	free(dictionary_remove(memtable,key));
}

void* hilodump(){

	while(1){
		sleep(config->tiempo_dump);

		dictionary_iterator(memtable,(void*)crearTemporal);
	}

	pthread_exit(NULL);
}


