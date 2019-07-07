/*
 * Funciones.h
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#ifndef FILE_SYSTEM_SRC_FUNCIONES_H_
#define FILE_SYSTEM_SRC_FUNCIONES_H_

#include "hiloClientes.h"
#include "buscar.h"

typedef struct {
	char * puerto;
	char * montaje;
	int retardo;
	int tam_value;
	int tiempo_dump;

}structConfig;


void inicializar();
void archivoDeConfiguracion(char* argv);
int leer_metadata();
int abrir_bitmap();
void finalizar();
void liberarTablas(st_tablaCompac * tabla);


#endif /* FILE_SYSTEM_SRC_FUNCIONES_H_ */
