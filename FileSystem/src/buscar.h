/*
 * buscar.h
 *
 *  Created on: 15 may. 2019
 *      Author: utnso
 */

#ifndef SRC_BUSCAR_H_
#define SRC_BUSCAR_H_


char * buscarKey(char * name, int key, int particion);
structRegistro * buscarEnLista(st_tabla * data, uint16_t key);
structRegistro * buscarEnParticion(char * path, uint16_t key);
structRegistro * buscarEnTemporales(char * name, uint16_t key);
structRegistro * buscarEnArchivo(char * path, uint16_t key);



#endif /* SRC_BUSCAR_H_ */
