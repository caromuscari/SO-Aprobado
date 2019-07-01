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
#include "configuracionMemoria.h"
#include <time.h>
#include <funcionesCompartidas/registroTabla.h>

typedef struct{
	char* nombreTabla;
	t_list* tablaDePaginas;
}st_segmento;

typedef struct{
	int nroDePagina;
	void* pagina;
	int flagModificado;
}st_tablaDePaginas;

typedef enum{
	LIBRE = 0,
	OCUPADO = 1
}t_condicion;

typedef struct{
	t_condicion condicion;
	double timestamp;
}st_marco;

void inicializarMemoria();

st_registro* comandoSelect(st_select* comandoSelect);

st_segmento* buscarSegmentoPorNombreTabla(char* nombreTabla);

st_tablaDePaginas* buscarPaginaPorKey(t_list* tablaDePaginas, uint16_t key);

int buscarMarcoLibre();

int algoritmoLRU();

#endif /* SRC_SEGMENTACIONPAGINADA_H_ */
