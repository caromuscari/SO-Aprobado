/*
 * buscar.h
 *
 *  Created on: 15 may. 2019
 *      Author: utnso
 */

#ifndef SRC_BUSCAR_H_
#define SRC_BUSCAR_H_

#include <commons/collections/list.h>
#include "manejoArchivos.h"
#include <funcionesCompartidas/listaMetadata.h>

typedef struct {
	sem_t compactacion;
	sem_t opcional;
	sem_t mutexC;
	int contador;
	int cont2;
	t_list * sem;
	st_metadata * meta;
	pthread_t hilo;

}st_tablaCompac;


char * buscarKey(char * name, int key, int particion);
structRegistro * buscarEnLista(st_tabla * data, uint16_t key);
structRegistro * buscarEnParticion(char * path, uint16_t key);
structRegistro * buscarEnTemporales(char * name, uint16_t key);
structRegistro * buscarEnArchivo(char * path, uint16_t key);
void leerTablas();

#endif /* SRC_BUSCAR_H_ */
