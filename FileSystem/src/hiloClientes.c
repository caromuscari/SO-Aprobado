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
#include <funcionesCompartidas/registroTabla.h>
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

void tratarCliente(cliente_t * cliente){

	int status=0;
	bool flag = true;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

	while(loop && flag){
		mensaje * recibido = malloc(sizeof(mensaje));
		int respuesta;
		char * buffer;
		size_t size;

		recibido->buffer = getMessage(cliente->socket, &(recibido->head), &status);

		if(recibido->buffer == NULL) flag=false;

		switch(recibido->head.codigo)
		{
			case INSERT:
				log_info(alog, "Recibi un Insert");
				st_insert * insert;
				insert = desserealizarInsert(recibido->buffer);

				buffer = strdup("");

				if(string_length(insert->value) <= config->tam_value)
				{
					respuesta = realizarInsert(insert);
					enviarRespuesta(respuesta, buffer, cliente->socket, &status, sizeof(buffer));

				}else{
					enviarRespuesta(3, buffer, cliente->socket, &status, sizeof(buffer));
				}

				destroyInsert(insert);
				free(buffer);
				break;

			case SELECT:
				log_info(alog, "Recibi un Select");
				st_select * selectt;
				char * registro;
				st_registro * reg;
				selectt = deserealizarSelect(recibido->buffer);

				respuesta = realizarSelect(selectt, &registro);
				reg = cargarRegistro(registro);
				buffer = serealizarRegistro(reg,&size);
				enviarRespuesta(respuesta, buffer, cliente->socket, &status, size);

				destoySelect(selectt);
				destroyRegistro(reg);
				free(registro);
				free(buffer);
				break;

			case CREATE:
				log_info(alog, "Recibi un Create");
				st_create * create;
				create = deserealizarCreate(recibido->buffer);

				respuesta = realizarCreate(create);
				actualizar_bitmap();

				buffer = strdup("");
				enviarRespuesta(respuesta, buffer, cliente->socket, &status, sizeof(buffer));

				destroyCreate(create);
				free(buffer);
				break;

			case DROP:
				log_info(alog, "Recibi un Drop");
				st_drop * drop;
				drop = deserealizarDrop(recibido->buffer);

				respuesta = realizarDrop(drop);
				actualizar_bitmap();

				buffer = strdup("");
				enviarRespuesta(respuesta, buffer, cliente->socket, &status, sizeof(buffer));

				destroyDrop(drop);
				free(buffer);
				break;

			case DESCRIBE:
				log_info(alog, "Recibi un Describe");
				st_describe * describe;
				st_metadata * meta;
				describe = deserealizarDescribe(recibido->buffer);

				respuesta = realizarDescribe(describe, &meta);

				buffer = serealizarMetaData(meta, &size);

				enviarRespuesta(respuesta, buffer, cliente->socket, &status, size);

				destroyDescribe(describe);
				free(buffer);
				break;

			case DESCRIBEGLOBAL:
				log_info(alog, "Recibi un Describe Global");

				respuesta = realizarDescribeGlobal();

				if(respuesta == 13){
					buffer = serealizarListaMetaData(listaTabla,&size);
					enviarRespuesta(respuesta, buffer, cliente->socket, &status,size);
					//list_clean(listaTabla);
					list_destroy(listaTabla);
				}else{
					enviarRespuesta(respuesta, buffer, cliente->socket, &status,sizeof(buffer));
				}

				free(buffer);
				break;
			default:
				flag = false;
				enviarRespuesta(16, buffer, cliente->socket, &status, sizeof(buffer)); //Modificar numero

		}

		free(recibido->buffer);
		free(recibido);

		sleep(config->retardo);
	}

	log_info(alog, "se desconecto el socket client %d",cliente->socket);
	close(cliente->socket);
	char * socketS = string_itoa(cliente->socket);
	free(dictionary_remove(clientes, socketS));
	free(socketS);

	pthread_exit(NULL);
}

void enviarRespuesta(int codigo, char * buffer, int socketC, int * status, size_t tam){

	header * head = malloc(sizeof(header));

	head->letra = 'F';
	head->codigo = codigo;
	head->sizeData = tam;

	message * mensaje = createMessage(head, buffer);

	enviar_message(socketC, mensaje, alog, status);

	free(head);
	free(mensaje->buffer);
	free(mensaje);
}

