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

char * getCriterioByEnum(TypeCriterio criterio);
TypeCriterio getTipoCriterioByString(char * criterioString);
#endif //KERNEL_UTILS_H
