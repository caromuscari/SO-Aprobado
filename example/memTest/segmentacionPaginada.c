
/*
 * segmentacionPaginada.c
 *
 *  Created on: 12 jun. 2019
 *      Author: jx
 */

//INSERT TABLA1 3 "juan" 12345678
//INSERT TABLA2 4 "giula" 12345678
//SELECT TABLA1 5 "ukraahre" 12345678


#include "segmentacionPaginada.h"



void comandoInsert(st_pagina *puntero,int numeroPagina,char *nombrePagina,char* valor){
	void * punteroValor = malloc(4);
	punteroValor = valor;

	(puntero + numeroPagina) -> nroDePagina = numeroPagina;
	(puntero + numeroPagina) -> nbrePagina = nombrePagina;
	(puntero + numeroPagina) -> direccionValor = punteroValor;

	//free(punteroValor);
}

void * comandoSelect(st_pagina tablaDePaginas[],int numeroPagina){
	printf("----------------------------\n" );
	printf("Numero de pagina: %i\n",tablaDePaginas[numeroPagina].nroDePagina );
	printf("Nombre de la pagina: %i\n",tablaDePaginas[numeroPagina].nbrePagina );
	printf("Dato de la pagina: %s\n",tablaDePaginas[numeroPagina].direccionValor );

}
