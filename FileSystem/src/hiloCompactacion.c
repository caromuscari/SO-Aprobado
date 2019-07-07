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
#include <commons/collections/queue.h>
#include "operaciones.h"
#include "hiloCompactacion.h"
#include "manejoArchivos.h"
#include <commons/bitarray.h>

extern t_dictionary * tablas;
extern int tBloques;
extern t_bitarray* bitmap;
extern t_log* alog;
extern t_queue * nombre;
extern int loop;

void hilocompactacion(){

	FILE* archivo;
	int i = 1, valor;
	char * nomTabla = queue_pop(nombre);
	printf("Hilo compactacion\n");
	printf("Nombre de tabla: %s\n", nomTabla);
	char * pathTabla;
	char * path, *new;
	t_dictionary * lista;
	st_tablaCompac * tabla = dictionary_get(tablas, nomTabla);

	free(nomTabla);

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

	while(loop){

		sleep(tabla->meta->compaction_time);

		pathTabla = armar_path(nomTabla);

		path = string_from_format("%s/%d.tmp", pathTabla, i);
		archivo = fopen(path,"r");

		while(archivo != NULL){
			new = string_from_format("%s/%d.tmpc", pathTabla, i);
			rename(path, new);

			i++;
			path = string_from_format("%s/%d.tmp", pathTabla, i);
			archivo = fopen(path,"r");
		}
		free(path);

		if(i != 1){
			lista = dictionary_create();
			for(int j=0;j<tabla->meta->partitions;j++){
				path = string_from_format("%s/%d.bin", pathTabla, j);
				dictionary_put(lista,string_itoa(j),llenarTabla(path));
				free(path);
			}

			for(int j=1;j<i;j++){
				char * path = string_from_format("%s/%d.tmpc", pathTabla, j);
				leerTemporal(path,lista, tabla->meta->partitions);
				free(path);
			}

			//Bloquear la tabla
			/*if(list_size(tabla->sem) != 0){
				sem_wait(&tabla->opcional);
				sem_wait(&tabla->compactacion);
			}else{
				log_info(alog, "Se bloquea la compactacion");
				sem_wait(&tabla->compactacion);
			}*/

			sem_wait(&tabla->compactacion);
			if(tabla->contador != 0) sem_wait(&tabla->opcional);

			eliminarTemporales(pathTabla);
			for(int j=0;j<tabla->meta->partitions;j++){
				eliminarParticion(pathTabla,j);
				generarParticion(pathTabla,j,lista);
			}
			//Desbloquear la tabla
			log_info(alog, "Se desbloquea la compactacion");
			sem_post(&tabla->compactacion);
			/*list_iterate(tabla->sem, (void*)desbloquear);
			list_clean(tabla->sem);*/

			sem_getvalue(&tabla->opcional, &valor);
			while(valor != 1){
				sem_post(&tabla->compactacion);
			}
		}
	}

	pthread_exit(NULL);
}

void desbloquear(sem_t *semaforo){
	sem_post(semaforo);
	log_info(alog, "Se desbloquea la tabla");
	sem_destroy(semaforo);
}

t_list * llenarTabla(char * path){
	t_list * lista = list_create();
	structParticion * particion;
	int i = 0;
	FILE * archivo;
	char * linea, * bloque;
	size_t tamBuffer = 100;
	char ** split;
	char * flag;

	particion = leerParticion(path);

	while(particion->bloques[i] != NULL){
		bloque = armar_PathBloque(particion->bloques[i]);
		archivo = fopen(path,"r");
		linea = malloc(sizeof(char) * tamBuffer);
		while(getline(&linea, &tamBuffer, archivo) != -1){
			structRegistro * reg;
			if(flag != NULL){
				string_append(&flag, linea);
				split = string_split(linea,";");
				reg = malloc(sizeof(structRegistro));
				reg->time = atol(split[0]);
				reg->key = atoi(split[1]);
				reg->value = strtok(split[2], "\n");

				list_add(lista, reg);

				free(flag);
			}else{
				split = string_split(linea,";");
				if(split[0] != NULL){
					if(split[1] == NULL) flag = strdup(linea);
					else if(split[2] != NULL && string_contains(split[2], "\n")){
						reg = malloc(sizeof(structRegistro));
						reg->time = atol(split[0]);
						reg->key = atoi(split[1]);
						reg->value = strtok(split[2], "\n");

						list_add(lista, reg);
					}else flag = strdup(linea);
				}else flag = strdup(linea);
			}

			string_iterate_lines(split, (void*)free);
			free(split);
			free(linea);

			linea = malloc(sizeof(char) * tamBuffer);
		}
		i++;
		fclose(archivo);
		free(bloque);
	}

	string_iterate_lines(particion->bloques, (void*)free);
	free(particion->bloques);
	free(particion);

	return lista;
}

void leerTemporal(char * path, t_dictionary * lista, int totalPart){
	FILE * archivo;
	char * bloque, *linea, ** split, * flag;
	size_t tamBuffer = 100;
	structParticion * particion;
	int i = 0, part, j=0, flag2=0;

	particion = leerParticion(path);

	while(particion->bloques[i] != NULL){
		bloque = armar_PathBloque(particion->bloques[i]);
		archivo = fopen(bloque,"r");
		linea = malloc(sizeof(char) * tamBuffer);
		while(getline(&linea, &tamBuffer, archivo) != -1){
			if(flag != NULL){
				string_append(&flag, linea);
				split = string_split(linea,";");

				part = atoi(split[1]) % totalPart;
				t_list * listPart = dictionary_get(lista,string_itoa(part));

				while(j<list_size(listPart)){
					structRegistro * reg = list_get(listPart, j);
					if(atoi(split[1]) == reg->key){
						if(atol(split[0]) > reg->time){
							reg->time = atol(split[0]);
							free(reg->value);
							reg->value = strtok(split[2], "\n");
						}
						flag2 = 1;
					}
					j++;
				}
				if(flag2 == 0){
					structRegistro * reg2 = malloc(sizeof(structRegistro));
					reg2->time = atol(split[0]);
					reg2->key = atoi(split[1]);
					reg2->value = strtok(split[2], "\n");

					list_add(listPart,reg2);
				}

				free(flag);
			}else{
				split = string_split(linea,";");
				if(split[0] != NULL){
					if(split[1] == NULL) flag = strdup(linea);
					else if(split[2] != NULL && string_contains(split[2], "\n")){
						part = atoi(split[1]) % totalPart;
						t_list * listPart = dictionary_get(lista,string_itoa(part));

						while(j<list_size(listPart)){
							structRegistro * reg = list_get(listPart, j);
							if(atoi(split[1]) == reg->key){
								if(atol(split[0]) > reg->time){
									reg->time = atol(split[0]);
									free(reg->value);
									reg->value = strtok(split[2], "\n");
								}
								flag2 = 1;
							}
							j++;
						}
						if(flag2 == 0){
							structRegistro * reg2 = malloc(sizeof(structRegistro));
							reg2->time = atol(split[0]);
							reg2->key = atoi(split[1]);
							reg2->value = strtok(split[2], "\n");

							list_add(listPart,reg2);
						}

					}else flag = strdup(linea);
				}else flag = strdup(linea);
			}
			j=0;

			string_iterate_lines(split, (void*)free);
			free(split);
			free(linea);

			linea = malloc(sizeof(char) * tamBuffer);
		}
		i++;
		fclose


		(archivo);
		free(bloque);

	}

	string_iterate_lines(particion->bloques, (void*)free);
	free(particion->bloques);
	free(particion);

}

void generarParticion(char * path, int part, t_dictionary * lista){
	char * pathPart = string_from_format("%s/%d.bin", path, part);
	FILE * archivo;
	int bit, i=0, offset=0;
	char * contenido,* registro, * bloque;
	t_list * listPart = dictionary_get(lista, string_itoa(part));

	bit = verificar_bloque();
	if(bit != -1){
		bitarray_set_bit(bitmap,bit);
		contenido = string_from_format("SIZE=0\nBLOQUES=[%d]", bit);

		archivo = fopen(pathPart, "a+");

		fputs(contenido,archivo);

		fclose(archivo);

		free(contenido);
	}

	bloque = armar_PathBloque(string_itoa(bit));
	archivo = fopen(bloque,"w");

	while(i<list_size(listPart)){
		structRegistro * reg = list_remove(listPart,i);
		registro = string_from_format("%d;%d;%s\n", reg->time, reg->key, reg->value);

		free(reg->value);
		free(reg);

		if((offset+string_length(registro)) > tBloques){
			fwrite(registro,sizeof(char),(tBloques-offset),archivo);
			registro += (tBloques-offset);

			bit = verificar_bloque();
			if(bit != -1){
				bitarray_set_bit(bitmap,bit);
				actualizar_bloques(pathPart,bit);
				actualizar_size(pathPart,(tBloques-offset));
			}
			offset = 0;

			fclose(archivo);
			free(bloque);
			bloque = armar_PathBloque(string_itoa(bit));
			archivo = fopen(bloque,"w");

			fwrite(registro,sizeof(char),string_length(registro),archivo);
			offset += string_length(registro);

			actualizar_size(pathPart, string_length(registro));

		}else{
			fwrite(registro,sizeof(char),string_length(registro),archivo);
			offset += string_length(registro);

			actualizar_size(pathPart, string_length(registro));
		}

		free(registro);
		i++;
	}

	fclose(archivo);
	free(bloque);
	free(pathPart);
	list_destroy(dictionary_remove(lista, string_itoa(part)));
}
