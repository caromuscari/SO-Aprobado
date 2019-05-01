/*
 * hiloMensajes.c
 *
 *  Created on: 18 abr. 2019
 *      Author: utnso
 */

#include "hiloClientes.h"

#include <pthread.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <semaphore.h>
#include <commons/collections/dictionary.h>


extern t_log* alog;
extern t_dictionary * clientes;

void tratarCliente(int socketC){

	int status=0;
	bool flag = true;

	while(flag){
		mensaje * recibido = malloc(sizeof(mensaje));
		int respuesta;
		char * buffer;

		recibido->buffer = getMessage(socketC, &(recibido->head), &status);

		if(recibido->buffer == NULL) flag=false;

		switch(recibido->head.codigo)
		{
			case 1:
				log_info(alog, "Recibi un Insert");
				structInsert * insert;

				insert = desserealizarInsert(recibido->buffer);
				if(insert != NULL){
					if(string_length(insert->value) <= config->tam_value)
					{
						respuesta = realizarInsert(insert);

						enviarRespuesta(respuesta, &buffer);

					}else{
						enviarRespuesta(3, &buffer);
					}
				}else{
					enviarRespuesta(1, &buffer);
				}
				free(insert->nameTable);
				free(insert->value);
				free(insert);
				free(buffer);
				break;

			case 2:
				log_info(alog, "Recibi un Select");
				structSelect * select;

				select = desserealizarSelect(recibido->buffer);

				if(select != NULL){
					respuesta = realizarSelect(select, &buffer);

					enviarRespuesta(respuesta, &buffer);

				}else{
					enviarRespuesta(1,&buffer);
				}

				free(select->nameTable);
				free(select);
				free(buffer);

				break;

			case 3:
				log_info(alog, "Recibi un Create");
				structCreate * create;

				create = desserealizarCreate(recibido->buffer);

				if(create != NULL){
					respuesta = realizarCreate(create);

					enviarRespuesta(respuesta, &buffer);

				}else{
					enviarRespuesta(1, &buffer);
				}

				free(create->nameTable);
				free(create->tipoConsistencia);
				free(create);
				free(buffer);
				break;

			case 4:
				log_info(alog, "Recibi un Drop");
				structDrop * drop;

				drop = desserealizarDrop(recibido->buffer);

				if(drop != NULL){
					respuesta = realizarDrop(drop);

					enviarRespuesta(respuesta, &buffer);

				}else{
					enviarRespuesta(1, &buffer);
				}

				free(drop->nameTable);
				free(drop);
				free(buffer);
				break;

			case 5: //describe
				log_info(alog, "Recibi un Describe");
				break;

			default:
				flag = false;
				enviarRespuesta(15, &buffer); //Modificar numero

				free(buffer);

		}

		free(recibido->buffer);
		free(recibido);
	}

	log_info(alog, "se desconecto el socket client %d",socketC);
	close(socketC);
	free(dictionary_remove(clientes, string_itoa(socketC)));

	pthread_exit(NULL);
}

