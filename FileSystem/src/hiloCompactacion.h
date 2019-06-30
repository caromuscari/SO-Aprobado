/*
 * hiloCompactacion.h
 *
 *  Created on: 20 jun. 2019
 *      Author: utnso
 */

#ifndef SRC_HILOCOMPACTACION_H_
#define SRC_HILOCOMPACTACION_H_


void hilocompactacion();
void desbloquear(sem_t *semaforo);
t_list * llenarTabla(char * path);
void leerTemporal(char * path, t_dictionary * lista, int totalPart);
void generarParticion(char * path, int part, t_dictionary * lista);


#endif /* SRC_HILOCOMPACTACION_H_ */
