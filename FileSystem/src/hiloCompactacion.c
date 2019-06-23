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
#include "operaciones.h"
#include "hiloCompactacion.h"

extern t_dictionary * tablas;

void hilocompactacion(char * name){

	signal(SIGKILL,senial);

	FILE* archivo;
	int i = 1;
	char * pathTabla = armar_path(name);
	char * path, *new;
	t_dictionary * lista;
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

		if(i != 1){
			lista = dictionary_create();
			for(int j=0;j<tabla->meta->partitions;j++){
				path = string_from_format("%s/%d.bin", path, j);
				dictionary_put(lista,string_itoa(j),llenarTabla(path));
				free(path);
			}

			for(int j=1;j<i;j++){
				leerTemporal(j,lista);
			}

			//Bloquear la tabla
			eliminarTemporales(pathTabla);
			for(int j=0;j<tabla->meta->partitions;j++){
				eliminarParticion(pathTabla,j);
				generarParticion(pathTabla,j,lista);
			}
			//Desbloquear la tabla
		}
	}

	pthread_exit(NULL);
}

t_list * llenarTabla(char * path){
	t_list * lista = list_create();
	structParticion * particion;
	int i = 0, flag = 0;
	FILE * archivo;
	char * linea;
	size_t tamBuffer = 100;

	particion = leerParticion(path);

	while(particion->bloques[i] != NULL){
		archivo = fopen(path,"r");
		linea = malloc(sizeof(char) * tamBuffer);
		while(getline(&linea, 0, archivo) != -1){
			if(flag != 0){

			}


		}


		i++;
	}

	return lista;
}

void leerTemporal(int temp, t_dictionary * lista){

}

void generarParticion(char * path, int part, t_dictionary * lista){

}
