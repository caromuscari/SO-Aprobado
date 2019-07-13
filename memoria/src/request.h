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


int mandarCreate(st_create * create);
int mandarDrop(st_drop * drop);
int mandarDescribe(st_describe * describe, st_metadata ** buff);
int mandarDescribeGlobal(t_list ** lista);
int mandarInsert(st_insert * insert);


#endif /* SRC_REQUEST_H_ */
