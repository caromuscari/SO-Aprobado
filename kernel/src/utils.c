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
    return CriterioNoDefinido;
}

char * getTipoCriterioBYEnum(TypeCriterio criterio){
    switch (criterio){
        case StrongConsistency:{
            return "SC";
        }
        case StrongHashConsistency:{
            return  "SHC";
        }
        case EventualConsistency:{
            return  "EC";
        }
        default:{
            return "No Definido";
        }
    }
}

char * getOperacionBYEnum(enum OPERACION type){
    switch (type){
        case INSERT:{
            return "INSERT";
        }
        case SELECT:{
            return "SELECT";
        }
        default: {
            return "NO Definido";
        }
    }
}

char * getNameTable(void * st_intrucion, enum OPERACION type){
    switch (type){
        case INSERT:{
            return strdup(((st_insert *)st_intrucion)->nameTable);
        }
        case SELECT:{
            return strdup(((st_select *)st_intrucion)->nameTable);
        }
        case DROP:{
            return strdup(((st_drop *)st_intrucion)->nameTable);
        }
        case DESCRIBE:{
            return strdup(((st_describe *)st_intrucion)->nameTable);
        }
        default:{
            return NULL;
        }
    }
}

char * getHashTable(void * st_intrucion, enum OPERACION type){
    switch (type){
        case INSERT:{
            char * hashM = strdup(((st_insert *) st_intrucion)->nameTable);
            char *keyString = string_itoa(((st_insert *) st_intrucion)->key);
            string_append(&hashM,keyString);
            return hashM;
        }
        case SELECT:{
            char * hashM = strdup(((st_select *) st_intrucion)->nameTable);
            char *keyString = string_itoa(((st_select *) st_intrucion)->key);
            string_append(&hashM,keyString);
            return hashM;
        }
        case DROP:{
            char * hashM = strdup(((st_drop *) st_intrucion)->nameTable);
            string_append(&hashM,"DROP");
            return hashM;
        }
        case DESCRIBE:{
            char * hashM = strdup(((st_describe *) st_intrucion)->nameTable);
            string_append(&hashM,"DESCRIBE");
            return hashM;
        }
        default:{
            return NULL;
        }
    }
}

char * generarTag(TypeCriterio tipo, void *st_intrucion, enum OPERACION type) {
    switch (tipo) {
        case StrongConsistency: {
            return getNameTable(st_intrucion, type);
        }
        case StrongHashConsistency: {
            return getHashTable(st_intrucion, type);
        }
        default: {
            return NULL;
        }
    }
}

int getCriterioBYInstruccion(void *st_intrucion, enum OPERACION type){
    switch (type){
        case INSERT:{
            return getCriterioByNameTabla(((st_insert *)st_intrucion)->nameTable);
        }
        case SELECT:{
            return getCriterioByNameTabla(((st_select *)st_intrucion)->nameTable);
        }
        case DROP:{
            return getCriterioByNameTabla(((st_drop *)st_intrucion)->nameTable);
        }
        case DESCRIBE:{
            return getCriterioByNameTabla(((st_describe *)st_intrucion)->nameTable);
        }
        default:{
            return  CriterioNoDefinido;
        }
    }
}