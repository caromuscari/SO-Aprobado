/*
 * hiloConsola.h
 *
 *  Created on: 20 abr. 2019
 *      Author: utnso
 */

#ifndef FILESYSTEM_SRC_HILOCONSOLA_H_
#define FILESYSTEM_SRC_HILOCONSOLA_H_

#include "manejoArchivos.h"

void* hiloconsola();
void mostrarRespuesta(int respuesta);
void mostrarTabla(st_metadata * meta);
void liberarMetadata(st_metadata * meta);

#endif /* FILESYSTEM_SRC_HILOCONSOLA_H_ */
