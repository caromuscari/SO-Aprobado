/*
 * hiloMensajes.h
 *
 *  Created on: 18 abr. 2019
 *      Author: utnso
 */

#ifndef FILE_SYSTEM_SRC_HILOMENSAJES_H_
#define FILE_SYSTEM_SRC_HILOMENSAJES_H_

#include <pthread.h>
#include <semaphore.h>
#include <funcionesCompartidas/funcionesNET.h>

typedef struct {
	header head;
	char* buffer;
}mensaje;

typedef struct {
	int socket;
	pthread_t hilo;
	sem_t semaforo;
}cliente_t;

void tratarCliente(int socketC);
void enviarRespuesta(int codigo, char ** buffer, int socketC, int * status);


#endif /* FILE_SYSTEM_SRC_HILOMENSAJES_H_ */
