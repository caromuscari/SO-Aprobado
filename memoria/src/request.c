/*
 * requests.c
 *
 *  Created on: 7 jul. 2019
 *      Author: utnso
 */

#include <funcionesCompartidas/API.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <stdio.h>
#include <stdlib.h>

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

char * mandarDescribe(st_describe * describe, int codigo){
	size_t size;
	header head, head2;
	message * mensaje;
	int controlador;
	char * buffer;

	if(codigo == 8){
		buffer = strdup("");
		size = sizeof(buffer);
	}else{
		buffer = serealizarDescribe(describe,&size);
	}

	head.letra = 'M';
	head.codigo = codigo;
	head.sizeData = size;

	mensaje = createMessage(&head, buffer);

	enviar_message(fdFileSystem, mensaje,file_log,&controlador);

	free(buffer);

	buffer = getMessage(fdFileSystem,&head2,&controlador);

	return buffer;
}


