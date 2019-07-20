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
#include <semaphore.h>
#include "Semaforos.h"

extern int tBloques;
extern t_log* alog;
extern t_queue * nombre;
extern sem_t sNombre;
extern int loop;

void hilocompactacion(){

	FILE* archivo;
	int i = 1, valor;
	sem_wait(&sNombre);
	char * nomTabla = queue_pop(nombre);
	sem_post(&sNombre);
	printf("Hilo compactacion\n");
	printf("Nombre de tabla: %s\n", nomTabla);
	char * pathTabla;
	char * path, *new;
	t_dictionary * lista;
	st_tablaCompac * tabla = leerDeTablas(nomTabla);


	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

	while(loop){

		sleep((tabla->meta->compaction_time)/1000);

		pathTabla = armar_path(nomTabla);

		path = string_from_format("%s/%d.tmp", pathTabla, i);
		archivo = fopen(path,"r");

		while(archivo != NULL){
			new = string_from_format("%s/%d.tmpc", pathTabla, i);
			rename(path, new);
			free(new);

			i++;
			free(path);
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

	free(nomTabla);

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
	char * flag = NULL;

	particion = leerParticion(path);

	while(particion->bloques[i] != NULL){
		bloque = armar_PathBloque(particion->bloques[i]);
		archivo = fopen(bloque,"r");
		linea = malloc(sizeof(char) * tamBuffer);
		while(getline(&linea, &tamBuffer, archivo) != -1){
			//if(linea != NULL){

			structRegistro * reg;
			if(flag != NULL){
				char * value = string_from_format("%s%s",flag,linea);
				//string_append(&flag, linea);
				split = string_split(value,";");
				if(split[0] != NULL){
					if(split[1] == NULL) flag = strdup(value);
					else if(split[2] != NULL && string_contains(split[2], "\n")){
						reg = malloc(sizeof(structRegistro));
						reg->time = strtod(split[0],NULL);
						reg->key = atoi(split[1]);
						reg->value = strdup(strtok(split[2], "\n"));

						list_add(lista, reg);

						free(flag);
						flag = NULL;
					}else flag = strdup(value);
				}else flag = strdup(value);
				free(value);
			}else{
				split = string_split(linea,";");
				if(split[0] != NULL){
					if(split[1] == NULL) flag = strdup(linea);
					else if(split[2] != NULL && string_contains(split[2], "\n")){
						reg = malloc(sizeof(structRegistro));
						reg->time = strtod(split[0],NULL);
						reg->key = atoi(split[1]);
						reg->value = strdup(strtok(split[2], "\n"));

						list_add(lista, reg);
					}else flag = strdup(linea);
				}else flag = strdup(linea);
			}

			string_iterate_lines(split, (void*)free);
			free(split);

			//}
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
	char * bloque, *linea, ** split, * flag = NULL;
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
				char * value = string_from_format("%s%s",flag, linea);
				//string_append(&flag, linea);
				split = string_split(value,";");
				if(split[0] != NULL){
					if(split[1] == NULL) flag = strdup(value);
					else if(split[2] != NULL && string_contains(split[2], "\n")){

						part = atoi(split[1]) % totalPart;
						t_list * listPart = dictionary_get(lista,string_itoa(part));

						while(j<list_size(listPart)){
							structRegistro * reg = list_get(listPart, j);
							if(atoi(split[1]) == reg->key){
								if(strtod(split[0],NULL) > reg->time){
									reg->time = strtod(split[0],NULL);
									free(reg->value);
									reg->value = strdup(strtok(split[2], "\n"));
								}
								flag2 = 1;
							}
							j++;
						}
						if(flag2 == 0){
							structRegistro * reg2 = malloc(sizeof(structRegistro));
							reg2->time = strtod(split[0],NULL);
							reg2->key = atoi(split[1]);
							reg2->value = strdup(strtok(split[2], "\n"));

							list_add(listPart,reg2);
						}
						free(flag);
					}else flag = strdup(value);
				}else flag = strdup(value);

				free(value);
				//flag =NULL;
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
								if(strtod(split[0],NULL) > reg->time){
									reg->time = strtod(split[0],NULL);
									free(reg->value);
									reg->value = strdup(strtok(split[2], "\n"));
								}
								flag2 = 1;
							}
							j++;
						}
						if(flag2 == 0){
							structRegistro * reg2 = malloc(sizeof(structRegistro));
							reg2->time = strtod(split[0],NULL);
							reg2->key = atoi(split[1]);
							reg2->value = strdup(strtok(split[2], "\n"));

							list_add(listPart,reg2);
						}

					}else flag = strdup(linea);
				}else if(!string_is_empty(linea)){
					flag = strdup(linea);
				}
			}
			j=0;

			string_iterate_lines(split, (void*)free);
			free(split);
			free(linea);

			linea = malloc(sizeof(char) * tamBuffer);
		}
		i++;
		fclose(archivo);
		free(linea);
		free(bloque);

	}

	string_iterate_lines(particion->bloques, (void*)free);
	free(particion->bloques);
	free(particion);

}
void generarParticion(char * path, int part, t_dictionary * lista){
    char * pathPart = string_from_format("%s/%d.bin", path, part);
    FILE * archivo;
    int* bit, i=0,sizeRestante;
    char * contenido,* registro, * bloque;
    t_list * listPart = dictionary_get(lista, string_itoa(part));
    t_list * listBits = list_create();

    char* strParticion = strdup("");
    while(i<list_size(listPart)) {
        structRegistro *reg = list_remove(listPart, i);
        registro = string_from_format("%f;%d;%s\n", reg->time, reg->key, reg->value);
        string_append(&strParticion, registro);

        free(reg->value);
        free(reg);
    }

    sizeRestante = string_length(strParticion);
    int cantidadCaracteresPorString = tBloques/4;
    int huboError = 0;
    while (sizeRestante > 0){
        bit = malloc(sizeof(int));
        *bit = verificar_bloque();
        if(*bit != -1){
            list_add(listBits, bit);
        }else{
            huboError = 1;
        }
        sizeRestante -= cantidadCaracteresPorString;
    }

    if(huboError == 0){
        sizeRestante = string_length(strParticion);
        char * stringBits = list_fold(listBits, strdup(""), (void*)armarStrBloques);
        contenido = string_from_format("SIZE=%d\nBLOQUES=[%s]", string_length(strParticion)*4, stringBits);
        archivo = fopen(pathPart, "a+");
        fputs(contenido,archivo);
        fclose(archivo);
        free(contenido);
        free(stringBits);

        int * siguienteBloque = list_get(listBits, 0);
        int iBloque = 0;
        while(sizeRestante > 0 && siguienteBloque != NULL){
            char* strBloque = string_substring(strParticion, cantidadCaracteresPorString * iBloque, cantidadCaracteresPorString);
            bloque = armar_PathBloque(string_itoa(*siguienteBloque));
            archivo = fopen(bloque,"w");
            fputs(strBloque, archivo);
            fclose(archivo);
            
            sizeRestante -= cantidadCaracteresPorString;
            free(strBloque);
            iBloque++;
            siguienteBloque = list_get(listBits, iBloque);
        }
    }else{
        list_iterate(listBits, (void*)liberarBit);
    }

    free(strParticion);
    free(pathPart);
    list_iterate(listBits, free);
    list_destroy(listBits);
    list_destroy(dictionary_remove(lista, string_itoa(part)));
}