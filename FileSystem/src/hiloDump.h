/*
 * hiloDump.h
 *
 *  Created on: 26 may. 2019
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include "operaciones.h"
#include "Funciones.h"
#include "manejoArchivos.h"
#include <semaphore.h>

#ifndef SRC_HILODUMP_H_
#define SRC_HILODUMP_H_


void* hilodump();
void crearTemporal(char * key, st_tabla* data);
char* armarStrLista(char *strLista, structRegistro *registro);
char* buscarNombreProximoTemporal(char* nombreTabla);


#endif /* SRC_HILODUMP_H_ */
