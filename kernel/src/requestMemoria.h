//
// Created by miguelchauca on 14/07/19.
//

#ifndef KERNEL_REQUESTMEMORIA_H
#define KERNEL_REQUESTMEMORIA_H

#include <funcionesCompartidas/API.h>
#include <funcionesCompartidas/listaMemoria.h>
#include "load_config.h"
#include <funcionesCompartidas/funcionesNET.h>
#include <funcionesCompartidas/registroTabla.h>
#include "threadMetadata.h"
#include "threadPoolMemoria.h"
#include "contracts.h"
#include <funcionesCompartidas/time.h>

int enviarRequestMemoria(st_instruccion *laInstruccion, st_memoria *datoMemoria);
st_messageResponse *consultarAMemoria(char *ip, char *puerto, int codigo, void *buffer, size_t sizeBuffer);
void destroyStMessageResponse(st_messageResponse *stMR);
#endif //KERNEL_REQUESTMEMORIA_H
