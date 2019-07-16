//
// Created by miguelchauca on 14/07/19.
//

#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include <string.h>

typedef enum {
    NoDefinido = -1,
    StrongConsistency = 1,
    StrongHashConsistency = 2,
    EventualConsistency = 3
} TypeCriterio;

typedef enum {
    SALIO_OK = 1,
    NO_SALIO_OK = 2,
    SE_DESCONECTO_SOCKET = 3
} Respuestas;

char * getCriterioByEnum(TypeCriterio criterio);
TypeCriterio getTipoCriterioByString(char * criterioString);
#endif //KERNEL_UTILS_H
