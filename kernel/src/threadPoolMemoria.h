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
#include "requestMemoria.h"
#include "contracts.h"
#include <funcionesCompartidas/time.h>

void *loadPoolMemori();
void updateListaMemorias(st_data_memoria * nuevaLista);
bool setTipoConsistencia(int number,TypeCriterio tipo);
void hacerJournal();
void eliminarMemoria(int numberMemoria);
st_memoria *getMemoria(TypeCriterio tipo, char *text);
int journalMemoria(st_memoria * memoria);
void addHistory(st_history_request * historyRequest, int numeroMemoria);
st_history_request * newHistory(enum OPERACION operacion, TypeCriterio criterio);
t_list * getHistoryByRange(double start, double end);

#endif //KERNEL_THREADPOOLMEMORIA_H
