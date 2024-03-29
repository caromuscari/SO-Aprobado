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
#include "Semaforos.h"


extern t_log* alog;
extern t_dictionary * clientes;
extern sem_t sClientes;
extern int loop;

void tratarCliente(cliente_t * cliente){

	int status=0;
	bool flag = true;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

	while(loop && flag){
		mensaje * recibido = malloc(sizeof(mensaje));
		int respuesta;
		char * buffer = strdup(" ");
		size_t size;

		recibido->buffer = getMessage(cliente->socket, &(recibido->head), &status);

		//if(recibido->buffer == NULL) flag=false;

		switch(recibido->head.codigo)
		{
			case INSERT:
				log_info(alog, "Recibi un Insert");
				st_insert * insert;
				insert = desserealizarInsert(recibido->buffer);

				if(string_length(insert->value) <= getValue())
				{
					respuesta = realizarInsert(insert);
					enviarRespuesta(respuesta, buffer, cliente->socket, &status, string_length(buffer)+1);

				}else{
					enviarRespuesta(3, buffer, cliente->socket, &status, string_length(buffer)+1);
				}

				destroyInsert(insert);
				free(buffer);
				break;

			case SELECT:
				log_info(alog, "Recibi un Select");
				st_select * selectt;
				char * registro = NULL;
				st_registro * reg;
				selectt = deserealizarSelect(recibido->buffer);

				respuesta = realizarSelect(selectt, &registro);
				if(registro != NULL){
					reg = cargarRegistro(registro);
                    log_info(alog, registro);
                    free(buffer);
                    buffer = serealizarRegistro(reg,&size);
				}else{
				    size = string_length(buffer)+1;
				}
				enviarRespuesta(respuesta, buffer, cliente->socket, &status, size);

				destoySelect(selectt);
				if(registro != NULL){
					destroyRegistro(reg);
					free(registro);
					free(buffer);
				}
				break;

			case CREATE:
				log_info(alog, "Recibi un Create");
				st_create * create;
				create = deserealizarCreate(recibido->buffer);

				respuesta = realizarCreate(create);
				//actualizar_bitmap();

				enviarRespuesta(respuesta, buffer, cliente->socket, &status, string_length(buffer)+1);

				destroyCreate(create);
				free(buffer);
				break;

			case DROP:
				log_info(alog, "Recibi un Drop");
				st_drop * drop;
				drop = deserealizarDrop(recibido->buffer);

				respuesta = realizarDrop(drop);
				//actualizar_bitmap();

				enviarRespuesta(respuesta, buffer, cliente->socket, &status, string_length(buffer)+1);

				destroyDrop(drop);
				free(buffer);
				break;

			case DESCRIBE:
				log_info(alog, "Recibi un Describe");
				st_describe * describe;
				st_metadata * meta;
				describe = deserealizarDescribe(recibido->buffer);

				respuesta = realizarDescribe(describe, &meta);

				if(respuesta == 15){
					free(buffer);
					buffer = serealizarMetaData(meta, &size);
                    enviarRespuesta(respuesta, buffer, cliente->socket, &status,size);
				}else{
                    enviarRespuesta(respuesta, buffer, cliente->socket, &status,string_length(buffer)+1);
				}

				destroyDescribe(describe);
                free(buffer);
				break;

			case DESCRIBEGLOBAL:
				log_info(alog, "Recibi un Describe Global");
				t_list * lista;

				respuesta = realizarDescribeGlobal(&lista);

				if(respuesta == 13){
					free(buffer);
					//mostrarTabla(list_get(lista,0));
					buffer = serealizarListaMetaData(lista,&size);
					enviarRespuesta(respuesta, buffer, cliente->socket, &status,size);
					//destroyListaMetaData(lista);
					//list_destroy(lista);
				}else{
					enviarRespuesta(respuesta, buffer, cliente->socket, &status,string_length(buffer)+1);
				}

				free(buffer);
				break;
			default:
				flag = false;

		}

		if(recibido->buffer != NULL) free(recibido->buffer);
		free(recibido);

		sleep(getRetardo());
	}

	log_info(alog, "se desconecto el socket client %d",cliente->socket);
	close(cliente->socket);
	char * socketS = string_itoa(cliente->socket);
	sem_wait(&sClientes);
	free(dictionary_remove(clientes, socketS));
	sem_post(&sClientes);
	free(socketS);

	pthread_exit(NULL);
}

void enviarRespuesta(int codigo, char * buffer, int socketC, int * status, size_t tam){

	header head;

	head.letra = 'F';
	head.codigo = codigo;
	head.sizeData = tam;

	message * mensaje = createMessage(&head, buffer);

	enviar_message(socketC, mensaje, alog, status);

	free(mensaje->buffer);
	free(mensaje);
}

