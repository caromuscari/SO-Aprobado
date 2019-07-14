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

int enviarRequestMemoria(stinstruccion *laInstruccion, st_memoria *datoMemoria);
#endif //KERNEL_REQUESTMEMORIA_H
