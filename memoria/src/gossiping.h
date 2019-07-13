//
// Created by miguelchauca on 13/07/19.
//

#ifndef MEMORIA_GOSSIPING_H
#define MEMORIA_GOSSIPING_H

#include <funcionesCompartidas/listaMemoria.h>
#include <commons/log.h>
#include "configuracionMemoria.h"
#include <pthread.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <funcionesCompartidas/codigoMensajes.h>

void *devolverListaMemoria(size_t *size_paquetes);
void *pthreadGossping();

#endif //MEMORIA_GOSSIPING_H
