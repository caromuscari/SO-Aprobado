//
// Created by miguelchauca on 28/04/19.
//

#ifndef KERNEL_THREADPOOLMEMORIA_H
#define KERNEL_THREADPOOLMEMORIA_H

#include <stdio.h>
#include <unistd.h>
#include <commons/collections/list.h>
#include "load_config.h"
#include <funcionesCompartidas/funcionesNET.h>
#include <funcionesCompartidas/listaMemoria.h>
#include <pthread.h>
#include <funcionesCompartidas/codigoMensajes.h>
#include "utils.h"
#include "threadPoolMemoria.h"

typedef struct {
    st_memoria * memoria;
    bool activo;
    t_list * tags;
    t_list * tipos;
    int count;
} st_kernel_memoria;
void *loadPoolMemori();
void updateListaMemorias(st_data_memoria * nuevaLista);
bool setTipoConsistencia(int number,TypeCriterio tipo);
st_memoria *getMemoria(TypeCriterio tipo, char *text);

#endif //KERNEL_THREADPOOLMEMORIA_H
