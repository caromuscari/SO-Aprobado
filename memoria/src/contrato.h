/*
 * contrato.h
 *
 *  Created on: 19 jul. 2019
 *      Author: utnso
 */

#ifndef SRC_CONTRATO_H_
#define SRC_CONTRATO_H_

#include <funcionesCompartidas/funcionesNET.h>

typedef enum{
	OK = 1,
	NOOK = 2,
	SOCKETDESCONECTADO= 3,
	FULLMEMORY = 4,
	MAYORQUEVALUEMAX = 5
}enum_resultados;

typedef struct {
	header cabezera;
	void *buffer;
} st_messageResponse;

void destroyStMessageResponse(st_messageResponse *stMR);

#endif /* SRC_CONTRATO_H_ */
