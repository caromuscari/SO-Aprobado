/*
 * comandos.h
 *
 *  Created on: 10 jul. 2019
 *      Author: utnso
 */

#ifndef SRC_COMANDOS_H_
#define SRC_COMANDOS_H_

#include "segmentacionPaginada.h"
#include "socketCliente.h"
#include <funcionesCompartidas/API.h>
#include "request.h"

void inicializarMemoria();

st_registro* comandoSelect(st_select* comandoSelect);

int comandoInsert(st_insert* comandoInsert);
st_registro* comandoSelect(st_select* comandoSelect);
int comandoJournal();

#endif /* SRC_COMANDOS_H_ */
