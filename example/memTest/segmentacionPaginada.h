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

#define cantidad 2

t_log * file_log;

typedef struct{
	char* nombreTabla;
	t_list tablaDePaginas;
}st_segmento;

typedef struct{
	int nroDePagina;
	void *direccionValor;
	char *nbrePagina;

}st_pagina;

t_list* listaDeSegmentos;
t_list* listaDePaginas;


void comandoInsert(st_pagina *puntero,int numeroPagina,char *nombrePagina,char* valor);
void * comandoSelect(st_pagina tablaDePaginas[],int numeroPagina);



#endif /* SRC_SEGMENTACIONPAGINADA_H_ */
