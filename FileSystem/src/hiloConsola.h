/*
 * hiloConsola.h
 *
 *  Created on: 20 abr. 2019
 *      Author: utnso
 */

#ifndef FILESYSTEM_SRC_HILOCONSOLA_H_
#define FILESYSTEM_SRC_HILOCONSOLA_H_

#include "manejoArchivos.h"
#include "hiloClientes.h"
#include "buscar.h"

void* hiloconsola();
void mostrarRespuesta(int respuesta);
void mostrarTabla(st_metadata * meta);
void liberarMetadata(st_metadata * meta);
void cerrarClientes(char * key, cliente_t * cliente);
void cerrarTablas(char * key, st_tablaCompac * tabla);

#endif /* FILESYSTEM_SRC_HILOCONSOLA_H_ */
