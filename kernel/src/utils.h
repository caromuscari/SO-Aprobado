//
// Created by miguelchauca on 14/07/19.
//

#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include <string.h>
#include <funcionesCompartidas/API.h>
#include "threadMetadata.h"
#include "contracts.h"

TypeCriterio getTipoCriterioByString(char * criterioString);
char * generarTag(TypeCriterio tipo, void *st_intrucion, enum OPERACION type);
TypeCriterio getCriterioBYInstruccion(void *st_intrucion, enum OPERACION type);
char * getTipoCriterioBYEnum(TypeCriterio criterio);
char * getOperacionBYEnum(enum OPERACION type);
#endif //KERNEL_UTILS_H
