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

extern int controlador;
extern int socketfs;
extern t_log* alog;
extern t_dictionary * clientes;
extern structConfig * config;

void * hiloselect(){

	controlador=0;

	signal(SIGKILL,senial);

	log_info(alog, "Se creo el hilo servidor");

	socketfs = makeListenSock(config->puerto, alog, &controlador);
	if(socketfs < 0) pthread_exit(NULL);

	log_info(alog, "Se creo el socket server");

	while(1)
	{
		int nuevo_socket = aceptar_conexion(socketfs, alog, &controlador);
		if(nuevo_socket != -1){

			mensaje * recibido = malloc(sizeof(mensaje));

			recibido->buffer = getMessage(nuevo_socket, &(recibido->head), &controlador);

			if(recibido->head.letra == 'M'){
				pthread_t hilo;
				cliente_t * cliente = malloc(sizeof(cliente_t));

				log_info(alog, "Se conecto un nuevo cliente");
				pthread_create(&hilo, NULL, (void*)tratarCliente,&cliente);
				pthread_detach(hilo);

				log_info(alog, "Se creo el hilo del cliente");

				cliente->socket = nuevo_socket;
				cliente->hilo = hilo;

				dictionary_put(clientes, string_itoa(nuevo_socket), cliente);

				char * value = string_itoa(config->tam_value);

				enviarRespuesta(0,&value, nuevo_socket, &controlador);

			}else{
				log_info(alog, "Se conecto un cliente incorrecto");
				close(nuevo_socket);
			}

			free(recibido->buffer);
			free(recibido);

		}
	}

	pthread_exit(NULL);
}

