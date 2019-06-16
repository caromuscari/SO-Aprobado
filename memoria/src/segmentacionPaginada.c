/*
 * segmentacionPaginada.c
 *
 *  Created on: 12 jun. 2019
 *      Author: utnso
 */

#include "segmentacionPaginada.h"

void inicializarMemoria(){
	memoriaPrincipal = malloc(1000); // aca voy a poner el tamaño segun el largo de value que me pase fs: ts+value+key
	listaDeSegmentos = list_create();
	log_info(file_log, "se creo la lista de segmentos");
}

