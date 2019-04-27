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

#include "hiloClientes.h"

extern int controlador;
extern int socketfs;
extern t_log* alog;
extern char* puerto;
extern t_dictionary * clientes;

void * hiloselect(){
	//sockets

	controlador=0;
	socketfs = makeListenSock(puerto, alog, &controlador);
	if(socketfs < 0) pthread_exit(NULL);

	while(1)//controlador ==6?
	{
		int nuevo_socket = aceptar_conexion(socketfs, alog, &controlador);
		if(nuevo_socket != -1){
			pthread_t hilo;
			cliente_t * cliente = malloc(sizeof(cliente_t));

			log_info(alog, "Se conecto un nuevo cliente");
			pthread_create(&hilo, NULL, (void*)tratarCliente,&cliente);
			pthread_detach(hilo);

			cliente->socket = nuevo_socket;
			cliente->hilo = hilo;

			dictionary_put(clientes, string_itoa(nuevo_socket), cliente);
		}

	}

	pthread_exit(NULL);
}



