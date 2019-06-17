
/*
 * segmentacionPaginada.c
 *
 *  Created on: 12 jun. 2019
 *      Author: jx
 */

#include "segmentacionPaginada.h"

#define cantidad 2


void cargarPagina(st_pagina *puntero,int numeroPagina,int flag,char* valor){
	void * punteroValor = malloc(4);
	punteroValor = valor;

	(puntero + numeroPagina) -> nroDePagina = numeroPagina;
	(puntero + numeroPagina) -> flagModificado = flag;
	(puntero + numeroPagina) -> direccionValor = punteroValor;

	//free(punteroValor);
}

void * mostrarPagina(st_pagina tablaDePaginas[],int numeroPagina){
	printf("----------------------------\n" );
	printf("Numero de pagina: %i\n",tablaDePaginas[numeroPagina].nroDePagina );
	printf("Flag: %i\n",tablaDePaginas[numeroPagina].flagModificado );
	printf("Dato de la pagina: %s\n",tablaDePaginas[numeroPagina].direccionValor );

}

int main(int argc, char const *argv[]) {

	st_pagina tablaDePaginas[cantidad], *punteroTabla;
	punteroTabla = tablaDePaginas;


	cargarPagina(punteroTabla,0,1,"hola");
	cargarPagina(punteroTabla,1,0,"chau");
	cargarPagina(punteroTabla,2,1,"azul");

	mostrarPagina(tablaDePaginas,1);
	mostrarPagina(tablaDePaginas,0);
	mostrarPagina(tablaDePaginas,2);
	mostrarPagina(tablaDePaginas,1);

	return 0;
}
