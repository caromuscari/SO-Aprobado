/*
 * Semaforos.c
 *
 *  Created on: 13 jul. 2019
 *      Author: utnso
 */

#include <commons/collections/dictionary.h>
#include <semaphore.h>
#include "manejoArchivos.h"
#include "buscar.h"
#include "Funciones.h"

extern sem_t sMemtable;
extern t_dictionary * memtable, * tablas;
extern structConfig * config;
extern sem_t sConfig, sTablas;


//Memtable
void agregarAMemtable(st_tabla * data, char* nameTable){
	sem_wait(&sMemtable);
	dictionary_put(memtable,nameTable, data);
	sem_post(&sMemtable);
}

void eliminarDeMemtable(char* nameTable){
	sem_wait(&sMemtable);
	free(dictionary_remove(memtable,nameTable));
	sem_post(&sMemtable);
}

st_tabla * leerDeMemtable(char* nameTable){
	st_tabla * data;
	sem_wait(&sMemtable);
	data = dictionary_get(memtable,nameTable);
	sem_post(&sMemtable);
	return data;
}

bool existeEnMemtable(char* nameTable){
	bool resp;
	sem_wait(&sMemtable);
	resp = dictionary_has_key(memtable,nameTable);
	sem_post(&sMemtable);
	return resp;
}


//Tablas
st_tablaCompac * leerDeTablas(char* nameTable){
	st_tablaCompac * data;
	sem_wait(&sTablas);
	data = dictionary_get(tablas,nameTable);
	sem_post(&sTablas);
	return data;
}

void agregarATablas(st_tablaCompac * data, char* nameTable){
	sem_wait(&sTablas);
	dictionary_put(tablas,nameTable, data);
	sem_post(&sTablas);
}

st_tablaCompac * eliminarDeTablas(char* nameTable){
	st_tablaCompac * data;
	sem_wait(&sTablas);
	data = dictionary_remove(tablas,nameTable);
	sem_post(&sTablas);
	return data;
}

bool existeEnTablas(char* nameTable){
	bool resp;
	sem_wait(&sTablas);
	resp = dictionary_has_key(tablas,nameTable);
	sem_post(&sTablas);
	return resp;
}

bool tablasVacia(){
	bool resp;
	sem_wait(&sTablas);
	resp = dictionary_is_empty(tablas);
	sem_post(&sTablas);
	return resp;
}

//Compactacion
int getContador(st_tablaCompac * tabla){
	sem_wait(&tabla->mutexC);
	int ret = tabla->cont2;
	sem_post(&tabla->mutexC);
	return ret;
}

void restarContador(st_tablaCompac * tabla){
	sem_wait(&tabla->mutexC);
	tabla->cont2--;
	sem_post(&tabla->mutexC);
}

void sumarContador(st_tablaCompac * tabla){
	sem_wait(&tabla->mutexC);
	tabla->cont2++;
	sem_post(&tabla->mutexC);
}

//Config
int getRetardo(){
	int retardo;
	sem_wait(&sConfig);
	retardo = config->retardo;
	sem_post(&sConfig);
	return retardo;
}

int getDump(){
	int dump;
	sem_wait(&sConfig);
	dump = config->tiempo_dump;
	sem_post(&sConfig);
	return dump;
}

int getValue(){
	int value;
	sem_wait(&sConfig);
	value = config->tam_value;
	sem_post(&sConfig);
	return value;
}

char* getPuerto(){
	char* value;
	sem_wait(&sConfig);
	value = config->puerto;
	sem_post(&sConfig);
	return value;
}

char* getMontaje(){
	char* value;
	sem_wait(&sConfig);
	value = config->montaje;
	sem_post(&sConfig);
	return value;
}
