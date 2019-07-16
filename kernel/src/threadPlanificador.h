//
// Created by miguelchauca on 04/05/19.
//

#ifndef KERNEL_PLANIFICADOR_H
#define KERNEL_PLANIFICADOR_H

#include <stdio.h>
#include <commons/collections/list.h>
#include <funcionesCompartidas/API.h>
#include <semaphore.h>
#include <funcionesCompartidas/log.h>
#include <unistd.h>
#include "load_config.h"
#include "requestMemoria.h"
#include "threadPoolMemoria.h"

void * inicialPlanificador();
void cargarNuevoScript(t_list * listInstrucciones);

#endif //KERNEL_PLANIFICADOR_H
