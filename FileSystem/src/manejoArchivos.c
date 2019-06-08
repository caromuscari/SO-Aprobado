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

extern t_dictionary * memtable;


int realizarInsert(st_insert * insert){

	int respuesta;
	st_metadata * metadata;
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
		registro->value = insert->value;

		metadata = leerMetadata(insert->nameTable); //Es necesario?
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

		free(metadata->consistency);
		free(metadata);
	}else{
		respuesta = 4;
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
			respuesta = 10;
		}

		free(metadata->consistency);
		free(metadata);
	}

	return respuesta;
}

int realizarCreate(st_create * create){
	int respuesta;
	char * path;
	int part;

	if(!validarArchivos(create->nameTable, &respuesta)){
			path = armar_path(create->nameTable);
			char * pathmkdir= string_from_format("sudo mkdir -p %s",path);
			system(pathmkdir);
			free(pathmkdir);

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

	if(validarArchivos(drop->nameTable, &respuesta)){
		metadata = leerMetadata(drop->nameTable);

		path = armar_path(drop->nameTable);

		for (int i = 1; i <= metadata->partitions; i++) {
			eliminarParticion(path, i);
		}

		eliminarDirectorio(path);

		respuesta = 9;

		free(path);
		free(metadata->consistency);
		free(metadata);
	}else{
		respuesta = 4;
	}


	return respuesta;
}

int realizarDescribe(st_describe * describe, char ** buffer){
	int respuesta;
	st_metadata * m;
	size_t size;

	if(validarArchivos(describe->nameTable, &respuesta)){
		m = leerMetadata(describe->nameTable);
		//*buffer = string_from_format("CONSISTENCY=%s\nPARTITIONS=%d\nCOMPACTION_TIME=%d", m->consistency, m->partitions, m->compaction_time);
		*buffer = serealizarMetaData(m, &size);
		respuesta = 13;
	}else{
		respuesta = 4;
	}

	return respuesta;
}

int realizarDescribeGlobal(char ** buffer){
	int respuesta;
	t_list* tablas;
	size_t size;

	tablas = listarDirectorio();

	if(tablas != NULL){
		*buffer = serealizarListaMetaData(tablas,&size);
		respuesta = 13;
	}else{
		respuesta = 12;
	}

	return respuesta;
}

bool validarArchivos(char * archivo, int * respuesta){
	FILE * file;
	char *path;

	path = armar_path(archivo);

	file = fopen(path,"r+");

	if(file == NULL){
		*respuesta = 4;
		free(path);
		return false;
	}

	fclose(file);
	free(path);

	return true;
}


