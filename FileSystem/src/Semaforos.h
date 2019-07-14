/*
 * Semaforos.h
 *
 *  Created on: 13 jul. 2019
 *      Author: utnso
 */

#ifndef SRC_SEMAFOROS_H_
#define SRC_SEMAFOROS_H_

#include "manejoArchivos.h"
#include "buscar.h"


void agregarAMemtable(st_tabla * data, char* nameTable);
void eliminarDeMemtable(char* nameTable);
st_tabla * leerDeMemtable(char* nameTable);
bool existeEnMemtable(char* nameTable);
st_tablaCompac * leerDeTablas(char* nameTable);
void agregarATablas(st_tablaCompac * data, char* nameTable);
st_tablaCompac * eliminarDeTablas(char* nameTable);
bool existeEnTablas(char* nameTable);
int getRetardo();
int getDump();
int getValue();
char* getPuerto();
char* getMontaje();


#endif /* SRC_SEMAFOROS_H_ */
