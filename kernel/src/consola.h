//
// Created by miguelchauca on 21/04/19.
//

#ifndef KERNEL_CONSOLA_H
#define KERNEL_CONSOLA_H

#include <readline/readline.h>
#include <commons/log.h>
#include <stdlib.h>
#include <funcionesCompartidas/API.h>
#include "load_config.h"
#include <commons/collections/list.h>
#include "threadPlanificador.h"
#include "threadMetadata.h"
#include "threadPoolMemoria.h"

typedef struct {
    int numero;
    TypeCriterio tipo;
} st_add_memoria;

void consola();

#endif //KERNEL_CONSOLA_H
