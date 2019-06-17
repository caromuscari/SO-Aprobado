/*
 * segmentacionPaginada.h
 *
 *  Created on: 12 jun. 2019
 *      Author: utnso
 */

#ifndef SRC_SEGMENTACIONPAGINADA_H_
#define SRC_SEGMENTACIONPAGINADA_H_

#include <commons/collections/list.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <funcionesCompartidas/log.h>

t_log * file_log;

typedef struct{
	char* nombreTabla;
	t_list tablaDePaginas;
}st_segmento;

typedef struct{
	int nroDePagina;
	void *direccionValor;
	int flagModificado;
	
}st_pagina;

t_list* listaDeSegmentos;
t_list* listaDePaginas;




#endif /* SRC_SEGMENTACIONPAGINADA_H_ */
