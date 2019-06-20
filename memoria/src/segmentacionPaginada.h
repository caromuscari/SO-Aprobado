/*
 * segmentacionPaginada.h
 *
 *  Created on: 12 jun. 2019
 *      Author: utnso
 */

#ifndef SRC_SEGMENTACIONPAGINADA_H_
#define SRC_SEGMENTACIONPAGINADA_H_

#include <commons/collections/list.h>
#include <funcionesCompartidas/API.h>
#include <stdlib.h>
#include <funcionesCompartidas/log.h>

typedef struct{
	char* nombreTabla;
	t_list* tablaDePaginas;
}st_segmento;

typedef struct{
	int nroDePagina;
	void* pagina;
	int flagModificado;
}st_tablaDePaginas;



void inicializarMemoria();

char* comandoSelect(st_select* comandoSelect);

st_segmento* buscarSegmentoPorNombreTabla(char* nombreTabla);

#endif /* SRC_SEGMENTACIONPAGINADA_H_ */
