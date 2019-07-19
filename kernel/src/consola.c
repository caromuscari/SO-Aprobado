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
        if (addMemoria->tipo == CriterioNoDefinido) {
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

void cargarScriptConUnaInstruccion(void *instruccion, enum OPERACION type, char *id) {
    t_list *listaDeInstrucciones = list_create();
    list_add(listaDeInstrucciones, crearInstruccion(instruccion, type));
    cargarNuevoScript(crearNuevoScript(id, listaDeInstrucciones));
}

char * getCleanLine(char * line){
    if(string_contains(line,"\n")){
        return string_substring(line,0,string_length(line) - 1);
    }else{
        return strdup(line);
    }
}

void cargarScriptFile(char *text) {
    char **split = string_split(text, " ");
    if(split[1] == NULL){
        printf("Verificar el comando ingresado\n");
        string_iterate_lines(split, (void *) free);
        free(split);
        return;
    }
    FILE *fileScript = fopen(split[1], "r");
    char *line;
    size_t len = 0;
    if (fileScript == NULL) {
        printf("no se puedo abrir el archivo\n");
        string_iterate_lines(split, (void *) free);
        free(split);
        return;
    }
    t_list *listaDeInstrucciones = list_create();
    bool flagError = false;
    char * lineClean = NULL;
    while (getline(&line, &len, fileScript) != -1 && !flagError) {
        lineClean = getCleanLine(line);
        int typeComando = getEnumFromString(lineClean);
        switch (typeComando) {
            case SELECT: {
                st_select *_select;
                if ((_select = cargarSelect(lineClean))) {
                    list_add(listaDeInstrucciones, crearInstruccion(_select, SELECT));
                } else {
                    flagError = true;
                }
                break;
            }
            case INSERT: {
                st_insert *_insert;
                if ((_insert = cargarInsert(lineClean))) {
                    list_add(listaDeInstrucciones, crearInstruccion(_insert, INSERT));
                } else {
                    flagError = true;
                }
                break;
            }
            case CREATE: {
                st_create *_create;
                if ((_create = cargarCreate(lineClean))) {
                    list_add(listaDeInstrucciones, crearInstruccion(_create, CREATE));
                } else {
                    flagError = true;
                }
                break;
            }
            case DROP: {
                st_drop *_drop;
                if ((_drop = cargarDrop(lineClean))) {
                    list_add(listaDeInstrucciones, crearInstruccion(_drop, DROP));
                } else {
                    flagError = true;
                }
                break;
            }
            case DESCRIBE:{
                st_describe *_describe = cargarDescribe(lineClean);
                if (_describe) {
                    list_add(listaDeInstrucciones, crearInstruccion(_describe, DESCRIBE));
                } else {
                    list_add(listaDeInstrucciones, crearInstruccion(_describe, DESCRIBEGLOBAL));
                }
                break;
            }
            default:  {
                flagError = true;
                break;
            }
        }
        free(lineClean);
    }
    free(line);
    fclose(fileScript);
    string_iterate_lines(split, (void *) free);
    free(split);
    if (flagError) {
        destroyListaInstruciones(listaDeInstrucciones);
        log_error(file_log,"No se reconocio algun comando del script");
    } else {
        cargarNuevoScript(crearNuevoScript(strdup("Script File"), listaDeInstrucciones));
    }
}

void armarComando(char *comando) {
    bool flagErrorSintaxis = false;
    int typeComando = getEnumFromString(comando);
    char *idText;
    switch (typeComando) {
        case INSERT: {
            st_insert *insert;
            flagErrorSintaxis = true;
            if ((insert = cargarInsert(comando))) {
                flagErrorSintaxis = false;
                if (getCriterioByNameTabla(insert->nameTable) != -1) {
                    idText = strdup("Script insert ");
                    string_append(&idText, insert->nameTable);
                    cargarScriptConUnaInstruccion(insert, INSERT, idText);
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
                if (getCriterioByNameTabla(_select->nameTable) != -1) {
                    idText = strdup("Script select ");
                    string_append(&idText, _select->nameTable);
                    cargarScriptConUnaInstruccion(_select, SELECT, idText);
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
                    idText = strdup("Script Drop ");
                    string_append(&idText, _drop->nameTable);
                    cargarScriptConUnaInstruccion(_drop, DROP, idText);
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
                idText = strdup("Script Create ");
                string_append(&idText, _create->nameTable);
                cargarScriptConUnaInstruccion(_create, CREATE, idText);
                flagErrorSintaxis = false;
            }
            break;
        }
        case DESCRIBE: {
            st_describe *_describe = cargarDescribe(comando);
            if (_describe) {
                idText = strdup("Script Describe ");
                string_append(&idText, _describe->nameTable);
                cargarScriptConUnaInstruccion(_describe, DESCRIBE, idText);
            } else {
                idText = strdup("Script Describe Global");;
                cargarScriptConUnaInstruccion(NULL, DESCRIBEGLOBAL, idText);
            }
            break;
        }
        case JOURNAL: {
            hacerJournal();
            break;
        }
        case RUN: {
            cargarScriptFile(comando);
            break;
        }
        case METRICS: {
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