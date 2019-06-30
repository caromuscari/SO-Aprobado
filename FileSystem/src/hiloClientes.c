/*
 * hiloMensajes.c
 *
 *  Created on: 18 abr. 2019
 *      Author: utnso
 */

#include "hiloClientes.h"

#include <pthread.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <funcionesCompartidas/API.h>
#include <semaphore.h>
#include <commons/collections/dictionary.h>
#include "Funciones.h"
#include "manejoArchivos.h"
#include "operaciones.h"
#include <stddef.h>
#include <signal.h>
#include "Funciones.h"
#include "hiloConsola.h"


extern t_log* alog;
extern t_dictionary * clientes;
extern structConfig * config;
extern t_list * listaTabla;
extern int loop;

void tratarCliente(int socketC){

	int status=0;
	bool flag = true;


	while(loop && flag){
		mensaje * recibido = malloc(sizeof(mensaje));
		int respuesta;
		char * buffer;
		size_t size;

		recibido->buffer = getMessage(socketC, &(recibido->head), &status);

		if(recibido->buffer == NULL) flag=false;

		switch(recibido->head.codigo)
		{
			case 1:
				log_info(alog, "Recibi un Insert");
				st_insert * insert;
				insert = desserealizarInsert(recibido->buffer);

				if(string_length(insert->value) <= config->tam_value)
				{
					respuesta = realizarInsert(insert);
					enviarRespuesta(respuesta, &buffer, socketC, &status, sizeof(buffer));

				}else{
					enviarRespuesta(3, &buffer, socketC, &status, sizeof(buffer));
				}

				destroyInsert(insert);
				//free(buffer);
				break;

			case 2:
				log_info(alog, "Recibi un Select");
				st_select * select;
				select = deserealizarSelect(recibido->buffer);

				respuesta = realizarSelect(select, &buffer);
				enviarRespuesta(respuesta, &buffer, socketC, &status, sizeof(buffer));

				destoySelect(select);
				free(buffer);
				break;

			case 3:
				log_info(alog, "Recibi un Create");
				st_create * create;
				create = deserealizarCreate(recibido->buffer);

				respuesta = realizarCreate(create);
				actualizar_bitmap();
				enviarRespuesta(respuesta, &buffer, socketC, &status, sizeof(buffer));

				destroyCreate(create);
				//free(buffer);
				break;

			case 4:
				log_info(alog, "Recibi un Drop");
				st_drop * drop;
				drop = deserealizarDrop(recibido->buffer);

				respuesta = realizarDrop(drop);
				actualizar_bitmap();
				enviarRespuesta(respuesta, &buffer, socketC, &status, sizeof(buffer));

				destroyDrop(drop);
				//free(buffer);
				break;

			case 5: //describe
				log_info(alog, "Recibi un Describe");
				st_describe * describe;
				st_metadata * meta;
				describe = deserealizarDescribe(recibido->buffer);

				respuesta = realizarDescribe(describe, &meta);

				buffer = serealizarMetaData(meta, &size);//Probar

				enviarRespuesta(respuesta, &buffer, socketC, &status, size);

				//liberarMetadata(meta);
				destroyDescribe(describe);
				free(buffer);
				break;

			case 6:
				log_info(alog, "Recibi un Describe Global");

				respuesta = realizarDescribeGlobal();

				buffer = serealizarListaMetaData(listaTabla,&size);
				enviarRespuesta(respuesta, &buffer, socketC, &status,size);//Enviar el size_t

				free(buffer);
				break;
			default:
				flag = false;
				enviarRespuesta(16, &buffer, socketC, &status, sizeof(buffer)); //Modificar numero

				//free(buffer);

		}

		free(recibido->buffer);
		free(recibido);

		sleep(config->retardo);
	}

	log_info(alog, "se desconecto el socket client %d",socketC);
	close(socketC);
	free(dictionary_remove(clientes, string_itoa(socketC)));

	pthread_exit(NULL);
}

void enviarRespuesta(int codigo, char ** buffer, int socketC, int * status, size_t tam){

	header * head = malloc(sizeof(header));

	head->letra = 'F';
	head->codigo = codigo;
	head->sizeData = tam;

	message * mensaje = createMessage(head, buffer);

	enviar_message(socketC, mensaje, alog, status);
}

