/*
 * manejoArchivos.h
 *
 *  Created on: 28 abr. 2019
 *      Author: utnso
 */

#ifndef SRC_MANEJOARCHIVOS_H_
#define SRC_MANEJOARCHIVOS_H_

#include <funcionesCompartidas/API.h>
#include <commons/collections/list.h>
#include <semaphore.h>
#include <stdint.h>
#include <funcionesCompartidas/listaMetadata.h>

typedef struct {
	int Size;
	char ** bloques;
}structParticion;

typedef struct {
	long int time;
	uint16_t key;
	char * value;
}structRegistro;

typedef struct {
	sem_t semaforo;
	t_list * lista;
}st_tabla;

int realizarInsert(st_insert * insert);
int realizarSelect(st_select * select, char ** value);
int realizarCreate(st_create * create);
int realizarDrop(st_drop * drop);
int realizarDescribe(st_describe * describe, st_metadata ** m);
int realizarDescribeGlobal();
bool validarArchivos(char * archivo, int* respuesta);
bool existeDirectorio(char * ruta, int * respuesta);
void liberarTabla(structRegistro * reg);


#endif /* SRC_MANEJOARCHIVOS_H_ */
