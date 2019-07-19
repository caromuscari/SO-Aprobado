/*
 * request.h
 *
 *  Created on: 7 jul. 2019
 *      Author: utnso
 */

#ifndef SRC_REQUEST_H_
#define SRC_REQUEST_H_

#include <funcionesCompartidas/listaMetadata.h>
#include "segmentacionPaginada.h"
#include "contrato.h"


int mandarCreate(st_create * create);
int mandarDrop(st_drop * drop);
st_messageResponse* mandarDescribe(st_describe * describe);
st_messageResponse* mandarDescribeGlobal();
int mandarInsert(st_insert * insert);
st_registro* obtenerSelect(st_select * comandoSelect);


#endif /* SRC_REQUEST_H_ */
