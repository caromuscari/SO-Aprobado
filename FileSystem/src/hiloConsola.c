/*
 * hiloConsola.c
 *
 *  Created on: 20 abr. 2019
 *      Author: utnso
 */


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/log.h>
#include <funcionesCompartidas/API.h>
#include "hiloConsola.h"
#include <string.h>
#include "manejoArchivos.h"
#include <commons/string.h>
#include "Funciones.h"
#include "operaciones.h"

extern t_log* alog;
extern structConfig * config;

void* hiloconsola(){
	//int flag = 1;

	char *request;
	char* ingreso;
	size_t tamBuffer = 100;
	int respuesta;

	log_info(alog, "Se creo el hilo consola");

	while(1){

		ingreso = malloc(sizeof(char) * tamBuffer);

		getline(&ingreso, &tamBuffer, stdin);

		request = strtok(ingreso, "\n");

		switch(getEnumFromString(request))
		{
			case 1:
				log_info(alog, "Recibi un Insert");
				st_insert * insert;
				insert = cargarInsert(request);

				if(insert != NULL){
					if(string_length(insert->value) <= config->tam_value)
					{
						respuesta = realizarInsert(insert);

						mostrarRespuesta(respuesta);

					}else{
						mostrarRespuesta(3);
					}
				}else{
					mostrarRespuesta(1);
				}

				destroyInsert(insert);
				break;

			case 2:
				log_info(alog, "Recibi un Select");
				st_select * select;
				char * key;
				select = cargarSelect(request);

				if(select != NULL){
					respuesta = realizarSelect(select, &key);

					if(respuesta != 10){//Revisar
						mostrarRespuesta(respuesta);
					}else{
						log_info(alog, key);
						printf("%s",key);
					}
				}else{
					mostrarRespuesta(1);
				}

				destoySelect(select);
				break;

			case 3:
				log_info(alog, "Recibi un Create");
				st_create * create;
				create = cargarCreate(request);

				if(create != NULL){
					respuesta = realizarCreate(create);
					actualizar_bitmap();
					mostrarRespuesta(respuesta);

				}else{
					mostrarRespuesta(1);
				}

				destroyCreate(create);
				break;

			case 4:
				log_info(alog, "Recibi un Drop");
				st_drop * drop;
				drop = cargarDrop(request);

				if(drop != NULL){
					respuesta = realizarDrop(drop);
					actualizar_bitmap();
					mostrarRespuesta(respuesta);

				}else{
					mostrarRespuesta(1);
				}

				destroyDrop(drop);
				break;

			case 5: //describe
				log_info(alog, "Recibi un Describe");
				st_describe * describe;
				char* buffer;
				describe = cargarDescribe(request);

				if(describe == NULL){
					respuesta = realizarDescribeGlobal(&buffer);
				}else{
					respuesta = realizarDescribe(describe,&buffer);
				}
				//Ver la respuesta
				mostrarRespuesta(respuesta);

				destroyDescribe(describe);
				break;

			default:
				mostrarRespuesta(2);

			}
		free(ingreso);

	}

	log_info(alog, "Sale del hilo consola");
	pthread_exit(NULL);
}


void mostrarRespuesta(int respuesta){

	switch(respuesta)
	{
		case 1:
			log_error(alog,"La estructura del request es incorrecta");
			break;
		case 2:
			log_error(alog,"El mensaje recibido es incorrecto");
			break;
		case 3:
			log_error(alog,"El value que se desea ingresar supera el tamaño maximo");
			break;
		case 4:
			log_error(alog,"La tabla ingresada no existe");
			break;
		case 5:
			log_info(alog,"El INSERT se realizo correctamente");
			break;
		case 6:
			log_error(alog,"La key ingresada no existe");
			break;
		case 7:
			log_error(alog,"La tabla ingresada ya existe");
			break;
		case 8:
			log_info(alog, "La tabla se creo correctamente");
			break;
		case 9:
			log_error(alog, "La tabla se elimino correctamente");
			break;
		case 10:
			log_error(alog, "No se pudo crear la tabla");
			break;
		case 11:
			log_error(alog, "No se pudo realizar el Insert");
			break;
		case 12:
			log_error(alog, "No se pudo realizar la request");
			break;

	}

}
