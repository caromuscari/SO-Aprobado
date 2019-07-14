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
extern t_list *listaDeMarcos;
extern t_list* listaDeSegmentos;

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

    st_segmento* segmentoEncontrado = buscarSegmentoPorNombreTabla(drop->nameTable);
    if(segmentoEncontrado){
        log_info(file_log, "Se encontro el segmento por Drop");

        for(int i = 0; i < list_size(segmentoEncontrado->tablaDePaginas); i++){
            st_tablaDePaginas* paginaDeTabla = list_get(segmentoEncontrado->tablaDePaginas, i);
            st_marco* marco = list_get(listaDeMarcos, paginaDeTabla->nroDePagina);
            free(paginaDeTabla);
            marco->condicion = LIBRE;
        }

        list_destroy(segmentoEncontrado->tablaDePaginas);
        free(segmentoEncontrado->nombreTabla);
        list_remove(listaDeSegmentos, segmentoEncontrado->nroSegmento);
        for(int i = segmentoEncontrado->nroSegmento; i < list_size(listaDeSegmentos); i++){
            st_segmento* segmento = list_get(listaDeSegmentos, i);
            int nro = segmento->nroSegmento;
            segmento->nroSegmento = nro - 1;
        }
    }

	buffer = serealizarDrop(drop,&size);

	head.letra = 'M';
	head.codigo = 4;
	head.sizeData = size;

	mensaje = createMessage(&head, buffer);

	enviar_message(fdFileSystem, mensaje,file_log,&controlador);
    if(controlador != 0){
        log_error(file_log, "no se pudo enviar el mensaje al FS");
        return -1;
    }

	free(buffer);

	buffer = getMessage(fdFileSystem,&head2,&controlador);
    if(controlador != 0){
        log_error(file_log, "no se pudo recibir un mensaje");
        return -1;
    }

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
    if(controlador != 0){
        log_error(file_log, "no se pudo enviar el mensaje al FS");
        return -1;
    }
    free(buffer);

    buffer = getMessage(fdFileSystem,&head2,&controlador);
    if(controlador < 0){
        log_error(file_log, "no se pudo recibir un mensaje");
        return -1;
    }

    free(buffer);

    return head2.codigo;
}




