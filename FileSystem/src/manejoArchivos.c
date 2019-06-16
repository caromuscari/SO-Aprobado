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
#include <errno.h>

extern t_dictionary * memtable;


int realizarInsert(st_insert * insert){

	int respuesta;
	st_tabla * data;

	structRegistro * registro;

	if(string_contains(insert->value,";")){
		respuesta = 1;
		return respuesta;
	}

	if(validarArchivos(insert->nameTable, &respuesta)){

		registro = malloc(sizeof(structRegistro *));
		registro->time = insert->timestamp;
		registro->key = insert->key;
		registro->value = string_new();
		string_append(&registro->value, insert->value);


		if(dictionary_has_key(memtable, insert->nameTable)){
			data = dictionary_get(memtable, insert->nameTable);

			sem_wait(&data->semaforo);
			list_add(data->lista, registro);
			sem_post(&data->semaforo);

		}else{
			data = malloc(sizeof(st_tabla));
			dictionary_put(memtable,insert->nameTable, data);

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
	st_metadata * metadata;
	int particion;

	if(validarArchivos(select->nameTable, &respuesta)){
		metadata = leerMetadata(select->nameTable);

		particion = select->key % metadata->partitions;

		*value = buscarKey(select->nameTable,select->key, particion);

		if(*value ==NULL){
			respuesta = 6;
		}else{
			respuesta = 14;
		}

		liberarMetadata(metadata);
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

			free(path);

			respuesta = 8;

	}else{
		respuesta = 7;
	}


	return respuesta;
}

int realizarDrop(st_drop * drop){
	int respuesta;
	st_metadata * metadata;
	char * path;
	st_tabla * data;

	if(validarArchivos(drop->nameTable, &respuesta)){
		metadata = leerMetadata(drop->nameTable);

		path = armar_path(drop->nameTable);

		if(dictionary_has_key(memtable, drop->nameTable)){
			data = dictionary_get(memtable, drop->nameTable);

			sem_wait(&data->semaforo);

			list_iterate(data->lista,(void*)liberarTabla);
			list_destroy(data->lista);

			sem_post(&data->semaforo);

			free(dictionary_remove(memtable,drop->nameTable));
		}

		for (int i = 0; i < metadata->partitions; i++) {
			eliminarParticion(path, i);
		}

		eliminarDirectorio(path);

		respuesta = 9;

		free(path);
		liberarMetadata(metadata);
	}

	return respuesta;
}

int realizarDescribe(st_describe * describe, st_metadata ** m){
	int respuesta;

	if(validarArchivos(describe->nameTable, &respuesta)){
		*m = leerMetadata(describe->nameTable);
		respuesta = 15;
	}

	return respuesta;
}

int realizarDescribeGlobal(t_list ** tablas){
	int respuesta;

	*tablas = listarDirectorio();

	if(tablas != NULL){
		respuesta = 13;
	}else{
		respuesta = 12;
	}

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


