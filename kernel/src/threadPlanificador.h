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

typedef struct {
    char * id;
    t_list * listaDeInstrucciones;
}st_script;

void * inicialPlanificador();
void cargarNuevoScript(st_script * newProceso);
st_script *crearNuevoScript(char *id, t_list *listaInstrucciones);
stinstruccion * crearInstruccion(void *instruccion, enum OPERACION type, TypeCriterio criterio);

#endif //KERNEL_PLANIFICADOR_H
