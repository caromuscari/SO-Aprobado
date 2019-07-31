/*
 * hiloSelect.c
 *
 *  Created on: 20 abr. 2019
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <funcionesCompartidas/log.h>
#include <pthread.h>
#include <commons/collections/dictionary.h>
#include "Funciones.h"
#include "hiloClientes.h"
#include <signal.h>
#include "Funciones.h"
#include <semaphore.h>
#include "Semaforos.h"

extern int controlador;
extern int socketfs;
extern t_log* alog;
extern t_dictionary * clientes;
extern sem_t sClientes;
extern int loop;

void * hiloselect(){

	controlador=0;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

	log_info(alog, "Se creo el hilo servidor");

	socketfs = makeListenSock(getPuerto(), alog, &controlador);
	if(socketfs < 0) pthread_exit(NULL);

	log_info(alog, "Se creo el socket server");

	while(loop)
	{
		int nuevo_socket = aceptar_conexion(socketfs, alog, &controlador);
		if(nuevo_socket != -1){

			mensaje * recibido = malloc(sizeof(mensaje));

			recibido->buffer = getMessage(nuevo_socket, &(recibido->head), &controlador);

			if(recibido->head.letra == 'M'){
				pthread_t hilo;
				cliente_t * cliente = malloc(sizeof(cliente_t));

				cliente->socket = nuevo_socket;

				log_info(alog, "Se conecto un nuevo cliente");
				pthread_create(&hilo, NULL, (void*)tratarCliente,cliente);
				pthread_detach(hilo);

				cliente->hilo = hilo;

				log_info(alog, "Se creo el hilo del cliente");

				char* socketN = string_itoa(nuevo_socket);
				sem_wait(&sClientes);
				dictionary_put(clientes, socketN, cliente);
				sem_post(&sClientes);
				free(socketN);

				char * value = string_itoa(getValue());

				enviarRespuesta(0,value, nuevo_socket, &controlador, string_length(value)+1);

				free(value);

			}else{
				log_info(alog, "Se conecto un cliente incorrecto");
				close(nuevo_socket);
			}

			free(recibido->buffer);
			free(recibido);

		}
	}

	close(socketfs);

	pthread_exit(NULL);
}

