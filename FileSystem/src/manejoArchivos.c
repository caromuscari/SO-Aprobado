/*
 * manejoArchivos.c
 *
 *  Created on: 28 abr. 2019
 *      Author: utnso
 */

#include "manejoArchivos.h"
#include <commons/collections/dictionary.h>
#include <commons/bitarray.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include "Funciones.h"
#include "operaciones.h"
#include "buscar.h"
#include <semaphore.h>
#include <funcionesCompartidas/listaMetadata.h>
#include <sys/stat.h>
#include <dirent.h>
#include "hiloConsola.h"
#include "hiloCompactacion.h"
#include <commons/collections/queue.h>
#include "Semaforos.h"

extern t_dictionary * tablas;
extern t_log* alog;
//extern t_queue * nombre;
extern sem_t /*sNombre,*/ sTablas;


int realizarInsert(st_insert * insert){

	int respuesta;
	st_tabla * data;
	structRegistro * registro;


	if(string_contains(insert->value,";")){
		respuesta = 1;
		return respuesta;
	}

	if(validarArchivos(insert->nameTable, &respuesta)){

		registro = malloc(sizeof(structRegistro));
		registro->time = insert->timestamp;
		registro->key = insert->key;
		registro->value = strdup(insert->value);

		log_info(alog,"Insert: %.0f;%d;%s\n", registro->time, registro->key, registro->value);

		if(existeEnMemtable(insert->nameTable)){
			data = leerDeMemtable(insert->nameTable);

			sem_wait(&data->semaforo);
			list_add(data->lista, registro);
			sem_post(&data->semaforo);

		}else{
			data = malloc(sizeof(st_tabla));

			agregarAMemtable(data, insert->nameTable);

			sem_init(&data->semaforo,0,1);
			data->lista = list_create();

			sem_wait(&data->semaforo);
			list_add(data->lista, registro);
			sem_post(&data->semaforo);
		}

		respuesta = 5;

	}

	return respuesta;
}

int realizarSelect(st_select * select, char ** value){

	int respuesta;
	int particion;
	st_tablaCompac * tabla;
	int *valor = malloc(sizeof(int));
	//sem_t *sem;

	if(validarArchivos(select->nameTable, &respuesta)){

		tabla = leerDeTablas(select->nameTable);

		/*sem_getvalue(&tabla->compactacion, &valor);
		if(valor != 1){
			sem= malloc(sizeof(sem_t));
			sem_init(sem,0,0);
			list_add(tabla->sem,sem);
			log_info(alog, "Se bloquea la tabla");
			sem_wait(sem);
		}else{
			sem= malloc(sizeof(sem_t));
			sem_init(sem,0,0);
			list_add(tabla->sem,sem);
		}*/

		sumarContador(tabla);
		sem_wait(&tabla->compactacion);
		sem_post(&tabla->compactacion);
		tabla->contador += 1;
		restarContador(tabla);

			particion = select->key % tabla->meta->partitions;

			*value = buscarKey(select->nameTable,select->key, particion);

			if(*value ==NULL){
				respuesta = 6;
			}else{
				respuesta = 14;
			}


		/*if(list_size(tabla->sem) == 1){
			sem_post(tabla->opcional);
		}*/

			tabla->contador -= 1;
			sem_getvalue(&tabla->opcional, valor);
			log_info(alog, "VALOR: %d", *valor);
			if(tabla->contador == 0 && *valor != 0){
				sem_post(&tabla->opcional);
			}
			free(valor);
	}

	return respuesta;
}

int realizarCreate(st_create * create){
	int respuesta;
	char * path;
	int part;

	if(!validarArchivos(create->nameTable, &respuesta)){
			path = armar_path(create->nameTable);
			mkdir(path, ACCESSPERMS);

			if(!crearMetadata(create,path)){
				eliminarDirectorio(path);
				free(path);
				respuesta = 10;
				return respuesta;
			}

			part = crearParticiones(create, path);
			if(part != -1){
				if(part != 0){
					for (int i = 0; i < part; i++) {
						eliminarParticion(path,i);
					}
				}
				eliminarDirectorio(path);
				free(path);
				respuesta = 10;
				return respuesta;
			}

			st_tablaCompac * tabla = malloc(sizeof(st_tablaCompac));
			tabla->meta = leerMetadata(create->nameTable);
			char * name = strdup(create->nameTable);

			sem_init(&tabla->compactacion,0,1);
			sem_init(&tabla->opcional,0,0);
			sem_init(&tabla->mutexC,0,1);
			tabla->contador = 0;
			tabla->cont2 = 0;

			//sem_wait(&sNombre);
			//queue_push(nombre, name);
			//sem_post(&sNombre);

			agregarATablas(tabla,create->nameTable);
			pthread_create(&tabla->hilo, NULL, (void*)hilocompactacion,name);
			pthread_detach(tabla->hilo);

			free(path);

			respuesta = 8;

	}else{
		respuesta = 7;
	}


	return respuesta;
}

int realizarDrop(st_drop * drop){
	int respuesta;
	char * path;
	st_tabla * data;
	st_tablaCompac * tabla;

	if(validarArchivos(drop->nameTable, &respuesta)){

		tabla = eliminarDeTablas(drop->nameTable);

		sem_wait(&tabla->compactacion);
		pthread_cancel(tabla->hilo);

		sem_destroy(&tabla->compactacion);
		sem_destroy(&tabla->opcional);

		path = armar_path(drop->nameTable);

		if(existeEnMemtable(drop->nameTable)){
			data = leerDeMemtable(drop->nameTable);

			sem_wait(&data->semaforo);

			list_iterate(data->lista,(void*)liberarTabla);
			list_destroy(data->lista);

			sem_post(&data->semaforo);
			sem_destroy(&data->semaforo);

			eliminarDeMemtable(drop->nameTable);
		}

		for (int i = 0; i < tabla->meta->partitions; i++) {
			eliminarParticion(path, i);
		}

		eliminarTemporales(path);

		eliminarTemporalesC(path);

		eliminarDirectorio(path);

		respuesta = 9;

		free(path);
		liberarMetadata(tabla->meta);
		free(tabla);
	}

	return respuesta;
}

int realizarDescribe(st_describe * describe, st_metadata ** m){
	int respuesta;

	if(existeEnTablas(describe->nameTable)){
		st_tablaCompac * tabla = leerDeTablas(describe->nameTable);
		*m = tabla->meta;
		respuesta = 15;
	}else respuesta = 4;

	return respuesta;
}

int realizarDescribeGlobal(t_list ** lista){
	int respuesta;


	void obtenerMetadatas(char * key, st_tablaCompac * tabla){

		st_metadata * meta = malloc(sizeof(st_metadata));
		meta->nameTable = strdup(tabla->meta->nameTable);
		meta->consistency = strdup(tabla->meta->consistency);
		meta->partitions = tabla->meta->partitions;
		meta->compaction_time = tabla->meta->compaction_time;
		list_add(*lista, meta);
	}

	if(!tablasVacia()){
		*lista = list_create();

		sem_wait(&sTablas);
		dictionary_iterator(tablas,(void*)obtenerMetadatas);
		sem_post(&sTablas);

		respuesta = 13;
	}else respuesta = 12;


	return respuesta;
}

bool validarArchivos(char * archivo, int * respuesta){
	char *path;
	DIR * directory;

	path = armar_path(archivo);

	directory = opendir(path);

	if(directory){
		closedir(directory);
	}else if(ENOENT == errno){
		*respuesta = 4;
		free(path);
		return false;
	}else{
		*respuesta = 12;
		free(path);
		return false;
	}

	free(path);

	return true;
}

bool existeDirectorio(char * ruta, int * respuesta){

    struct stat s;
    char* path = string_new();
    path = strdup(armar_path(ruta));
    int err = stat(path, &s);
    if(-1 != err) {

        if(S_ISDIR(s.st_mode)) {
            return true;
        }
    }
    free(path);
    *respuesta = 4;
    return false;
}

void liberarTabla(structRegistro * reg){
	free(reg->value);
	free(reg);
}


