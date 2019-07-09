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

enum TypeCriterio {
    StrongConsistency = 1,
    StrongHashConsistency = 2,
    EventualConsistency = 3
};
typedef struct {
    st_memoria * memoria;
    bool activo;
    t_list * tags;
    t_list * tipos;
    int count;
} st_kernel_memoria;
void *loadPoolMemori();
void updateListaMemorias(st_data_memoria * nuevaLista);
bool setTipoConsistencia(int number, enum TypeCriterio tipo);

#endif //KERNEL_THREADPOOLMEMORIA_H
