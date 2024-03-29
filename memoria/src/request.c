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
#include "contrato.h"
#include "console.h"
#include "hiloInotify.h"

extern int fdFileSystem;
extern t_log * file_log;

int mandarCreate(st_create * create){
	size_t size;
	header request, respuesta;
	message * mensaje;
	int controlador;
	char * buffer = serealizarCreate(create,&size);

	request.letra = 'M';
	request.codigo = CREATE;
	request.sizeData = size;

	mensaje = createMessage(&request, buffer);

	enviar_message(fdFileSystem, mensaje,file_log,&controlador);

	free(mensaje->buffer);
	free(mensaje);

	if(controlador != 0){
		log_error(file_log, "No se pudo enviar el mensaje");
		sleep(getRetardoFile()/1000);
		return NOOK;
	}

	free(buffer);

	buffer = getMessage(fdFileSystem,&respuesta,&controlador);

	if(buffer == NULL){
		log_error(file_log, "Fallo la conexion con el File System");
		sleep(getRetardoFile()/1000);
		return SOCKETDESCONECTADO;
	}

	free(buffer);
	sleep(getRetardoFile()/1000);
	return respuesta.codigo;
}

int mandarDrop(st_drop * drop){
	size_t size;
	header request, respuesta;
	message * mensaje;
	int controlador;
	char * buffer;

	buffer = serealizarDrop(drop,&size);

	request.letra = 'M';
	request.codigo = 4;
	request.sizeData = size;

	mensaje = createMessage(&request, buffer);

	enviar_message(fdFileSystem, mensaje,file_log,&controlador);
	free(mensaje->buffer);
	free(mensaje);
	free(buffer);
    if(controlador != 0){
        log_error(file_log, "No se pudo enviar el mensaje al File System");
        sleep(getRetardoFile()/1000);
        return NOOK;
    }

	buffer = getMessage(fdFileSystem,&respuesta,&controlador);
    if(buffer == NULL){
        log_error(file_log, "Fallo la conexion con File System");
        sleep(getRetardoFile()/1000);
        return SOCKETDESCONECTADO;
    }

	free(buffer);
	sleep(getRetardoFile()/1000);
	return respuesta.codigo;
}

st_registro* obtenerSelect(st_select * comandoSelect){
    int control = 0;
    header request;
    char* paqueteDeRespuesta;
    header respuesta;
    request.letra = 'M';
    request.codigo = SELECT;

    size_t size;
    void* paqueteDatos = serealizarSelect(comandoSelect, &size);

    request.sizeData = size;

    message* mensaje = createMessage(&request, paqueteDatos);
    enviar_message(fdFileSystem, mensaje, file_log, &control);
    free(paqueteDatos);
    free(mensaje->buffer);
    free(mensaje);
    if(control != 0){
        log_error(file_log, "No se pudo enviar el mensaje del select");
        sleep(getRetardoFile()/1000);
        return NULL;
    }

    paqueteDeRespuesta = getMessage(fdFileSystem, &respuesta, &control);
    if(paqueteDeRespuesta== NULL){
        log_error(file_log, "Fallo la conexion con el File System");
        sleep(getRetardoFile()/1000);
        return NULL;
    }
    if(respuesta.codigo == 14){
    	st_registro * reg = deserealizarRegistro(paqueteDeRespuesta);
    	free(paqueteDeRespuesta);
    	sleep(getRetardoFile()/1000);
        return reg;
    } else {
    	free(paqueteDeRespuesta);
    	sleep(getRetardoFile()/1000);
        return NULL;
    }
}

st_messageResponse* mandarDescribe(st_describe * describe){
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

	free(mensaje->buffer);
	free(mensaje);
	free(buffer);

	if(controlador != 0){
		log_error(file_log, "No se pudo enviar el mensaje");
		sleep(getRetardoFile()/1000);
		return NULL;
	}

	buffer = getMessage(fdFileSystem,&head2,&controlador);

	if(buffer == NULL){
		log_error(file_log, "Se desconecto file system");
		sleep(getRetardoFile()/1000);
		return NULL;
	}

	st_messageResponse* mensajeResp = malloc(sizeof(st_messageResponse));
	mensajeResp->cabezera.codigo = head2.codigo;
	mensajeResp->cabezera.letra = head2.letra;
	mensajeResp->cabezera.sizeData = head2.sizeData;

	mensajeResp->buffer = buffer;
	sleep(getRetardoFile()/1000);
	return mensajeResp;
}

st_messageResponse* mandarDescribeGlobal(){
	header request, respuesta;
	message * mensaje;
	int controlador;
	void* buffer;

	request.letra = 'M';
	request.codigo = DESCRIBEGLOBAL;
	request.sizeData = 1;

	buffer = strdup("1");

	mensaje = createMessage(&request, buffer);

	enviar_message(fdFileSystem, mensaje,file_log,&controlador);

	free(mensaje->buffer);
	free(mensaje);
	free(buffer);

	if(controlador != 0){
		log_error(file_log, "No se pudo enviar el mensaje");
		sleep(getRetardoFile()/1000);
		return NULL;
	}

	buffer = getMessage(fdFileSystem,&respuesta,&controlador);
	if(buffer == NULL){
		log_error(file_log, "Se desconecto file system");
		sleep(getRetardoFile()/1000);
        return NULL;
	}

	st_messageResponse* mensajeResp = malloc(sizeof(st_messageResponse));
		mensajeResp->cabezera.codigo = respuesta.codigo;
		mensajeResp->cabezera.letra = respuesta.letra;
		mensajeResp->cabezera.sizeData = respuesta.sizeData;

		mensajeResp->buffer = buffer;
		sleep(getRetardoFile()/1000);
		return mensajeResp;
}

int mandarInsert(st_insert * insert){
    size_t size;
    header head, head2;
    message * mensaje;
    int controlador;
    char * buffer;

    buffer = serealizarInsert(insert,&size);

    head.letra = 'M';
    head.codigo = INSERT;
    head.sizeData = size;

    mensaje = createMessage(&head, buffer);

    enviar_message(fdFileSystem, mensaje,file_log,&controlador);

    free(mensaje->buffer);
    free(mensaje);
    free(buffer);

    if(controlador != 0){
    	printf("No se pudo enviar el mensaje al File System\n");
        log_error(file_log, "No se pudo enviar el mensaje al File System");
        sleep(getRetardoFile()/1000);
        return -1;
    }

    buffer = getMessage(fdFileSystem,&head2,&controlador);
    if(buffer == NULL){
        log_error(file_log, "Se desconecto file system");
        sleep(getRetardoFile()/1000);
        return -1;
    }
    free(buffer);
    sleep(getRetardoFile()/1000);
    return head2.codigo;
}




