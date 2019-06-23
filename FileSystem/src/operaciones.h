/*
 * operaciones.h
 *
 *  Created on: 1 may. 2019
 *      Author: utnso
 */

#ifndef SRC_OPERACIONES_H_
#define SRC_OPERACIONES_H_

#include "manejoArchivos.h"
#include "archivos.h"
#include <funcionesCompartidas/listaMetadata.h>
#include <commons/collections/dictionary.h>

st_metadata * leerMetadata(char * archivo);
int verificar_bloque();
char * armar_path(char * archivo);
bool crearMetadata(st_create * c, char * path);
int crearParticiones(st_create * c, char * path);
void eliminarTemporales(char * path);
void eliminarDirectorio(char *path);
void eliminarParticion(char *path, int particion);
structParticion * leerParticion(char * path);
void actualizar_bitmap();
char * armar_PathBloque(char * bloque);
void actualizar_Particion(structActualizar * a);
t_dictionary * listarDirectorio();
structRegistro * leerBloque(char* bloque, uint16_t key, char ** flag);
int crearArchivoTemporal(char * pathCompleto);


#endif /* SRC_OPERACIONES_H_ */
