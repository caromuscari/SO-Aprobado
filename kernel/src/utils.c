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

char * generarTag(TypeCriterio tipo, void *st_intrucion, enum OPERACION type) {
    switch (tipo) {
        case StrongConsistency: {
            if (type == INSERT) {
                return strdup(((st_insert *) st_intrucion)->nameTable);
            } else {
                return strdup(((st_select *) st_intrucion)->nameTable);
            }
        }
        case StrongHashConsistency: {
            if (type == INSERT) {
                char * hashM = strdup(((st_insert *) st_intrucion)->nameTable);
                char *keyString = string_itoa(((st_insert *) st_intrucion)->key);
                string_append(&hashM,keyString);
                return hashM;
            } else {
                char * hashM = strdup(((st_select *) st_intrucion)->nameTable);
                char *keyString = string_itoa(((st_select *) st_intrucion)->key);
                string_append(&hashM,keyString);
                return hashM;
            }
        }
        default: {
            return NULL;
        }
    }
}