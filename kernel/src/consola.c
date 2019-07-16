//
// Created by miguelchauca on 21/04/19.
//
#include "consola.h"

extern t_log *file_log;
extern config *configuracion;

st_add_memoria *cargarAddMemoria(char *text) {
    st_add_memoria *addMemoria = malloc(sizeof(st_add_memoria));
    bool flagErro = false;
    char **split = string_split(text, " ");
    if (split[2]) {
        addMemoria->numero = atoi(split[2]);
    } else {
        flagErro = true;
    }
    if (split[4]) {
        addMemoria->tipo = getTipoCriterioByString(split[4]);
        if (addMemoria->tipo == NoDefinido) {
            flagErro = true;
        }
    } else {
        flagErro = true;
    }
    string_iterate_lines(split, (void *) free);
    free(split);
    if (flagErro) {
        free(addMemoria);
        return NULL;
    }
    return addMemoria;
}

char *getText(TypeCriterio tipo, void *st_intrucion, enum OPERACION type) {
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

void crearListInstrucciones(void *instruccion, enum OPERACION type, TypeCriterio criterio) {
    stinstruccion *newInstruccion = malloc(sizeof(stinstruccion));
    newInstruccion->operacion = type;
    newInstruccion->instruccion = instruccion;
    newInstruccion->criteio = criterio;
    newInstruccion->tag = getText(criterio, instruccion, type);
    t_list *listInstrucciones = list_create();
    list_add(listInstrucciones, newInstruccion);
    cargarNuevoScript(listInstrucciones);
}

void armarComando(char *comando) {
    bool flagErrorSintaxis = false;
    int typeComando = getEnumFromString(comando);
    TypeCriterio criterio = -1;
    switch (typeComando) {
        case INSERT: {
            st_insert *insert;
            flagErrorSintaxis = true;
            if ((insert = cargarInsert(comando))) {
                flagErrorSintaxis = false;
                if ((criterio = getCriterioByNameTabla(insert->nameTable)) != -1) {
                    crearListInstrucciones(insert, INSERT, criterio);
                    log_info(file_log, "EJECUTANDO COMANDO INSERT");
                } else {
                    log_error(file_log, "Tabla no encontrada");
                    destroyInsert(insert);
                }
            }
            break;
        }
        case SELECT: {
            st_select *_select;
            flagErrorSintaxis = true;
            if ((_select = cargarSelect(comando))) {
                flagErrorSintaxis = false;
                if ((criterio = getCriterioByNameTabla(_select->nameTable)) != -1) {
                    crearListInstrucciones(_select, SELECT, criterio);
                    log_info(file_log, "Ejecutando comando SELECT");
                } else {
                    log_error(file_log, "Tabla no encontrada");
                    destoySelect(_select);
                }
            }
            break;
        }
        case DROP: {
            st_drop *_drop;
            flagErrorSintaxis = true;
            if ((_drop = cargarDrop(comando))) {
                flagErrorSintaxis = false;
                if (getCriterioByNameTabla(_drop->nameTable) != -1) {
                    crearListInstrucciones(_drop, DROP, NoDefinido);
                    log_info(file_log, "EJECUTANDO COMANDO DROP");
                } else {
                    log_error(file_log, "Tabla no encontrada");
                    destroyDrop(_drop);
                }
            }
            break;
        }
        case CREATE: {
            st_create *_create;
            flagErrorSintaxis = true;
            if ((_create = cargarCreate(comando))) {
                crearListInstrucciones(_create, CREATE, NoDefinido);
                flagErrorSintaxis = false;
                log_info(file_log, "EJECUTANDO COMANDO CREATE");
            }
            break;
        }
        case DESCRIBE:{
            st_describe * _describe = cargarDescribe(comando);
            flagErrorSintaxis = true;
            if(_describe){
                log_info(file_log, "EJECUTANDO COMANDO DESCRIBE");
                crearListInstrucciones(_describe,DESCRIBE,NoDefinido);
            }else{
                log_info(file_log, "EJECUTANDO COMANDO DESCRIBE GLOBAL");
                crearListInstrucciones(NULL,DESCRIBE,NoDefinido);
            }
            break;
        }
        case JOURNAL:{
            hacerJournal();
            break;
        }
        case RUN:{
            break;
        }
        case METRICS:{
            break;
        }
        case ADD: {
            st_add_memoria *memoria = cargarAddMemoria(comando);
            if (memoria) {
                if (setTipoConsistencia(memoria->numero, memoria->tipo)) {
                    printf("se asigno correctamente el criterio\n");
                }
                free(memoria);
            } else {
                printf("revisar los parametros\n");
            }
            break;
        }
        default: {
            log_error(file_log, "Comando no reconocido");
        }
    }

    if (flagErrorSintaxis) {
        log_error(file_log, "Verificar el comando ingresado");
    }
}

void consola() {
    char *comando;
    comando = readline(">");
    printf("Ingrese comando LQL\n");
    string_trim(&comando);
    while (strcmp(comando, "exit") != 0) {
        armarComando(comando);
        free(comando);
        comando = readline(">");
        string_trim(&comando);
    }
    free(comando);
}