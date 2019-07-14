//
// Created by miguelchauca on 14/07/19.
//

#include "utils.h"

TypeCriterio getTipoCriterioByString(char * criterioString){
    if(strcmp(criterioString, "SC") == 0){
        return StrongConsistency;
    }
    if(strcmp(criterioString, "SHC") == 0){
        return StrongHashConsistency;
    }
    if(strcmp(criterioString,"EC") == 0){
        return EventualConsistency;
    }
    return NoDefinido;
}

char * getCriterioByEnum(TypeCriterio criterio){
    switch (criterio){
        case StrongConsistency:{
            return strdup("SC");
        }
        case StrongHashConsistency:{
            return strdup("SCH");
        }
        case EventualConsistency:{
            return strdup("EV");
        }
        default:{
            NULL;
        }
    }
}