/*
 * requests.c
 *
 *  Created on: 7 jul. 2019
 *      Author: utnso
 */

#include <funcionesCompartidas/API.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <funcionesCompartidas/listaMetadata.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "request.h"

extern int fdFileSystem;
extern t_log * file_log;

int mandarCreate(st_create * create){
	size_t size;
	header head, head2;
	message * mensaje;
	int controlador;
	char * buffer = serealizarCreate(create,&size);

	head.letra = 'M';
	head.codigo = 3;
	head.sizeData = size;

	mensaje = createMessage(&head, buffer);

	enviar_message(fdFileSystem, mensaje,file_log,&controlador);

	free(buffer);

	buffer = getMessage(fdFileSystem,&head2,&controlador);

	free(buffer);

	return head2.codigo;
}

int mandarDrop(st_drop * drop){
	size_t size;
	header head, head2;
	message * mensaje;
	int controlador;
	char * buffer;

	//Eliminar de memtable

	buffer = serealizarDrop(drop,&size);

	head.letra = 'M';
	head.codigo = 4;
	head.sizeData = size;

	mensaje = createMessage(&head, buffer);

	enviar_message(fdFileSystem, mensaje,file_log,&controlador);

	free(buffer);

	buffer = getMessage(fdFileSystem,&head2,&controlador);

	free(buffer);

	return head2.codigo;
}

int mandarDescribe(st_describe * describe, st_metadata ** buff){
	size_t size;
	header head, head2;
	message * mensaje;
	int controlador;
	char * buffer;

	buffer = serealizarDescribe(describe,&size);

	head.letra = 'M';
	head.codigo = DESCRIBE;
	head.sizeData = size;

	mensaje = createMessage(&head, buffer);

	enviar_message(fdFileSystem, mensaje,file_log,&controlador);

	free(buffer);

	buffer = getMessage(fdFileSystem,&head2,&controlador);

	if(head2.codigo == 15) *buff = deserealizarMetaData(buffer,&head2.sizeData);

	return head2.codigo;
}

int mandarDescribeGlobal(t_list ** lista){
	header head, head2;
	message * mensaje;
	int controlador;
	char * buffer = strdup("");

	head.letra = 'M';
	head.codigo = DESCRIBEGLOBAL;
	head.sizeData = sizeof(buffer);

	mensaje = createMessage(&head, buffer);

	enviar_message(fdFileSystem, mensaje,file_log,&controlador);

	free(buffer);

	buffer = getMessage(fdFileSystem,&head2,&controlador);

	if(head2.codigo == 13) *lista = deserealizarListaMetaData(buffer,head2.sizeData);

	free(buffer);
	return head2.codigo;
}
