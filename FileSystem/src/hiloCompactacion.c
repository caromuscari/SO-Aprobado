/*
 * hiloCompactacion.c
 *
 *  Created on: 20 jun. 2019
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "Funciones.h"
#include <unistd.h>
#include <commons/collections/dictionary.h>

extern t_dictionary * tablas;

void hilocompactacion(char * name){

	signal(SIGKILL,senial);

	FILE* archivo;
	int i = 1;
	char * tabla = armar_path(name);
	char * path, *new;
	t_list * lista;
	st_tablaCompac * tabla = dictionary_get(tablas, name);

	while(1){
		sleep(tabla->meta->compaction_time);

		path = string_from_format("%s/%d.tmp", path, i);
		archivo = fopen(path,"r");

		while(archivo != NULL){
			new = string_from_format("%s/%d.tmpc", path, i);
			rename(path, new);

			i++;
			path = string_from_format("%s/%d.tmp", path, i);
			archivo = fopen(path,"r");
		}
		free(path);

		for(int j=1;j<i;j++){

		}


	}

	pthread_exit(NULL);
}

