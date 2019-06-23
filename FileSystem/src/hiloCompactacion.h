/*
 * hiloCompactacion.h
 *
 *  Created on: 20 jun. 2019
 *      Author: utnso
 */

#ifndef SRC_HILOCOMPACTACION_H_
#define SRC_HILOCOMPACTACION_H_


void hilocompactacion(char * name);
t_list * llenarTabla(char * path);
void leerTemporal(int temp, t_dictionary * lista);
void generarParticion(char * path, int part, t_dictionary * lista);


#endif /* SRC_HILOCOMPACTACION_H_ */
