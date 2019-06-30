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
#include <commons/collections/dictionary.h>
#include <signal.h>
#include "Funciones.h"

extern t_log* alog;
extern structConfig * config;
extern t_list * listaTabla;
extern int loop;

void* hiloconsola(){

	char *request;
	char* ingreso;
	size_t tamBuffer = 100;
	int respuesta;


	log_info(alog, "Se creo el hilo consola");

	while(loop){

		printf("Ingrese una Request:\n");
		ingreso = malloc(sizeof(char) * tamBuffer);

		getline(&ingreso, &tamBuffer, stdin);

		request = strtok(ingreso, "\n");

		switch(getEnumFromString(request))
		{
			case 1:
				log_info(alog, "Request de tipo INSERT");
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
				log_info(alog, "Request de tipo SELECT");
				st_select * select;
				char * value;
				select = cargarSelect(request);

				if(select != NULL){
					respuesta = realizarSelect(select, &value);

					if(respuesta != 14){
						mostrarRespuesta(respuesta);
					}else{
						log_info(alog, value);
						printf("Value: %s\n",value);
					}
				}else{
					mostrarRespuesta(1);
				}

				destoySelect(select);
				break;

			case 3:
				log_info(alog, "Request de tipo CREATE");
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
				log_info(alog, "Request de tipo DROP");
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

			case 5:
				log_info(alog, "Request de tipo DESCRIBE");
				st_describe * describe;
				st_metadata * meta;
				describe = cargarDescribe(request);

				if(describe == NULL){
					respuesta = realizarDescribeGlobal();

					mostrarRespuesta(respuesta);
					list_iterate(listaTabla,(void*)mostrarTabla);
					list_destroy(listaTabla);
				}else{
					respuesta = realizarDescribe(describe,&meta);

					mostrarRespuesta(respuesta);
					if(respuesta == 15){
						mostrarTabla(meta);
					}

					destroyDescribe(describe);
				}

				break;

			case 6:
				loop = 0;
				break;
			default:
				mostrarRespuesta(2);

			}
		free(ingreso);

		//sleep(config->retardo);
	}

	log_info(alog, "Sale del hilo consola");
	pthread_exit(NULL);
}


void mostrarRespuesta(int respuesta){

	switch(respuesta)
	{
		case 1:
			log_error(alog,"La estructura del request es incorrecta");
			printf("La estructura del request es incorrecta\n");
			break;
		case 2:
			log_error(alog,"El mensaje recibido es incorrecto");
			printf("El mensaje recibido es incorrecto\n");
			break;
		case 3:
			log_error(alog,"El value que se desea ingresar supera el tamaño maximo");
			printf("El value que se desea ingresar supera el tamaño maximo\n");
			break;
		case 4:
			log_error(alog,"La tabla ingresada no existe");
			printf("El tabla ingresada no existe\n");
			break;
		case 5:
			log_info(alog,"El INSERT se realizo correctamente");
			printf("El INSERT se realizo correctamente\n");
			break;
		case 6:
			log_error(alog,"La key ingresada no existe");
			printf("La key ingresada no existe\n");
			break;
		case 7:
			log_error(alog,"La tabla ingresada ya existe");
			printf("La tabla ingresada ya existe\n");
			break;
		case 8:
			log_info(alog, "La tabla se creo correctamente");
			printf("La tabla se creo correctamente\n");
			break;
		case 9:
			log_error(alog, "La tabla se elimino correctamente");
			printf("La tabla se elimino correctamente\n");
			break;
		case 10:
			log_error(alog, "No se pudo crear la tabla");
			printf("No se pudo crear la tabla\n");
			break;
		case 11:
			log_error(alog, "No se pudo realizar el Insert");
			printf("No se pudo realizar el Insert\n");
			break;
		case 12:
			log_error(alog, "No se pudo realizar la request");
			printf("No se pudo realizar la request\n");
			break;
		case 13:
			log_info(alog, "Describe de tablas encontradas");
			printf("Describe de tablas encontradas\n");
			break;
		case 15:
			log_info(alog, "Describe de tabla");
			printf("Describe de tabla\n");
			break;

	}

}

void mostrarTabla(st_metadata * meta){
		printf("Table: %s\nConsistency: %s\nPartitions: %d\nCompaction Time: %d\n",meta->nameTable, meta->consistency, meta->partitions, meta->compaction_time);
}

void liberarMetadata(st_metadata * meta){
	free(meta->nameTable);
	free(meta->consistency);
	free(meta);
}
