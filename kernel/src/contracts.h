//
// Created by miguelchauca on 18/07/19.
//

#ifndef KERNEL_CONTRACTS_H
#define KERNEL_CONTRACTS_H

#include <commons/collections/list.h>
#include <funcionesCompartidas/API.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <funcionesCompartidas/listaMemoria.h>
#include <funcionesCompartidas/listaMetadata.h>
#include "../../funcionesCompartidas/funcionesCompartidas/API.h"

typedef enum {
    CriterioNoDefinido = -1,
    StrongConsistency = 1,
    StrongHashConsistency = 2,
    EventualConsistency = 3
} TypeCriterio;

typedef enum {
    SALIO_OK = 1,
    NO_SALIO_OK = 2,
    SE_DESCONECTO_SOCKET = 3,
    MEMORIA_NO_DISPONIBLE = 4,
    NO_SE_ENCONTRO_TABLA = 5,
    NO_HAY_RESULTADO_EN_SELECT = 6,
} Respuestas;

typedef struct {
    char *id;
    t_list *listaDeInstrucciones;
} st_script;

typedef struct {
    enum OPERACION operacion;
    void *instruccion;
} st_instruccion;


typedef struct {
    header cabezera;
    void *buffer;
} st_messageResponse;

typedef struct {
    enum OPERACION operacion;
    double startTime;
    double endTime;
    TypeCriterio tipoCriterio;
} st_history_request;

typedef struct {
    st_memoria *memoria;
    bool activo;
    t_list *tags;
    t_list *tipos;
    t_list * history;
    int count;
} st_kernel_memoria;

typedef struct {
    int numeroMemoria;
    t_list * history;
    t_list * tipos;
} st_metrica_memoria;

#endif //KERNEL_CONTRACTS_H
