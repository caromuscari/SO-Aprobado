//
// Created by miguelchauca on 21/04/19.
//
#include "consola.h"

extern t_log *file_log;
//extern config *configuracion;

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
    log_info(file_log, "[Console] cargando nuevo Script");
    t_list *listaDeInstrucciones = list_create();
    list_add(listaDeInstrucciones, crearInstruccion(instruccion, type));
    cargarNuevoScript(crearNuevoScript(id, listaDeInstrucciones));
    //free(id);
}

char *getCleanLine(char *line) {
    if (string_contains(line, "\n")) {
        return string_substring(line, 0, string_length(line) - 1);
    } else {
        return strdup(line);
    }
}

void cargarScriptFile(char *text) {
    char **split = string_split(text, " ");
    if (split[1] == NULL) {
        printf("Verificar el comando ingresado\n");
        string_iterate_lines(split, (void *) free);
        free(split);
        return;
    }
    FILE *fileScript = fopen(split[1], "r");
    char *line;
    size_t len = 100;
    if (fileScript == NULL) {
        printf("no se puedo abrir el archivo\n");
        string_iterate_lines(split, (void *) free);
        free(split);
        return;
    }
    t_list *listaDeInstrucciones = list_create();
    bool flagError = false;
    char *lineClean = NULL;
    line = malloc(sizeof(char) *len);
    while (getline(&line, &len, fileScript) != -1 && !flagError) {
        lineClean = getCleanLine(line);
        free(line);
        int typeComando = getEnumFromString(lineClean);
        switch (typeComando) {
            case SELECT: {
                log_info(file_log, "[load-Script] cargando Select");
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
                log_info(file_log, "[load-Script] cargando Insert");
                if ((_insert = cargarInsert(lineClean))) {
                    list_add(listaDeInstrucciones, crearInstruccion(_insert, INSERT));
                } else {
                    flagError = true;
                }
                break;
            }
            case CREATE: {
                st_create *_create;
                log_info(file_log, "[load-Script] cargando Create");
                if ((_create = cargarCreate(lineClean))) {
                    list_add(listaDeInstrucciones, crearInstruccion(_create, CREATE));
                } else {
                    flagError = true;
                }
                break;
            }
            case DROP: {
                st_drop *_drop;
                log_info(file_log, "[load-Script] cargando Drop");
                if ((_drop = cargarDrop(lineClean))) {
                    list_add(listaDeInstrucciones, crearInstruccion(_drop, DROP));
                } else {
                    flagError = true;
                }
                break;
            }
            case DESCRIBE: {
                st_describe *_describe = cargarDescribe(lineClean);
                if (_describe) {
                    log_info(file_log, "[load-Script] cargando Describe");
                    list_add(listaDeInstrucciones, crearInstruccion(_describe, DESCRIBE));
                } else {
                    log_info(file_log, "[load-Script] cargando Describe global");
                    list_add(listaDeInstrucciones, crearInstruccion(_describe, DESCRIBEGLOBAL));
                }
                break;
            }
            default: {
                log_info(file_log, "[load-Script] no se reconocio la operacion");
                flagError = true;
                break;
            }
        }
        free(lineClean);
        line = malloc(sizeof(char)* len);
    }
    free(line);
    fclose(fileScript);
    string_iterate_lines(split, (void *) free);
    free(split);
    if (flagError || listaDeInstrucciones->elements_count == 0) {
        destroyListaInstruciones(listaDeInstrucciones);
        if(listaDeInstrucciones->elements_count == 0){
            printf("no se puedo cargar ningun script\n");
        }
        log_error(file_log, "[load-Script] No se reconocio algun comando del script");
    } else {
        cargarNuevoScript(crearNuevoScript("Script File", listaDeInstrucciones));
    }
}

void armarComando(char *comando) {
    bool flagErrorSintaxis = false;
    int typeComando = getEnumFromString(comando);
    char *idText;
    switch (typeComando) {
        case INSERT: {
            log_info(file_log, "[Console] Operacion Insert");
            st_insert *insert;
            flagErrorSintaxis = true;
            if ((insert = cargarInsert(comando))) {
                flagErrorSintaxis = false;
                if (getCriterioByNameTabla(insert->nameTable) != -1) {
                    idText = strdup("Script insert ");
                    string_append(&idText, insert->nameTable);
                    cargarScriptConUnaInstruccion(insert, INSERT, idText);
                    free(idText);
                } else {
                    log_error(file_log, "Tabla no encontrada");
                    destroyInsert(insert);
                }
            }
            break;
        }
        case SELECT: {
            st_select *_select;
            log_info(file_log, "[Console] Operacion Select");
            flagErrorSintaxis = true;
            if ((_select = cargarSelect(comando))) {
                flagErrorSintaxis = false;
                if (getCriterioByNameTabla(_select->nameTable) != -1) {
                    idText = strdup("Script select ");
                    string_append(&idText, _select->nameTable);
                    cargarScriptConUnaInstruccion(_select, SELECT, idText);
                    free(idText);
                } else {
                    log_error(file_log, "Tabla no encontrada");
                    destoySelect(_select);
                }
            }
            break;
        }
        case DROP: {
            st_drop *_drop;
            log_info(file_log, "[Console] Operacion Drop");
            flagErrorSintaxis = true;
            if ((_drop = cargarDrop(comando))) {
                flagErrorSintaxis = false;
                if (getCriterioByNameTabla(_drop->nameTable) != -1) {
                    idText = strdup("Script Drop ");
                    string_append(&idText, _drop->nameTable);
                    cargarScriptConUnaInstruccion(_drop, DROP, idText);
                    free(idText);
                } else {
                    log_error(file_log, "Tabla no encontrada");
                    destroyDrop(_drop);
                }
            }
            break;
        }
        case CREATE: {
            st_create *_create;
            log_info(file_log, "[Console] Operacion Create");
            flagErrorSintaxis = true;
            if ((_create = cargarCreate(comando))) {
                idText = strdup("Script Create ");
                string_append(&idText, _create->nameTable);
                cargarScriptConUnaInstruccion(_create, CREATE, idText);
                flagErrorSintaxis = false;
                free(idText);
            }
            break;
        }
        case DESCRIBE: {
            st_describe *_describe = cargarDescribe(comando);
            if (_describe) {
                log_info(file_log, "[Console] Operacion Describe");
                idText = strdup("Script Describe ");
                string_append(&idText, _describe->nameTable);
                cargarScriptConUnaInstruccion(_describe, DESCRIBE, idText);
            } else {
                log_info(file_log, "[Console] Operacion Describe Global");
                idText = strdup("Script Describe Global");;
                cargarScriptConUnaInstruccion(NULL, DESCRIBEGLOBAL, idText);
            }
            free(idText);
            break;
        }
        case JOURNAL: {
            log_info(file_log, "[Console] Operacion Journal");
            hacerJournal();
            break;
        }
        case RUN: {
            log_info(file_log, "[Console] Operacion Run");
            cargarScriptFile(comando);
            break;
        }
        case METRICS: {
            break;
        }
        case ADD: {
            log_info(file_log, "[Console] Operacion ADD");
            st_add_memoria *memoria = cargarAddMemoria(comando);
            if (memoria) {
                if (setTipoConsistencia(memoria->numero, memoria->tipo)) {
                    printf("se asigno correctamente el criterio\n");
                }else{
                    printf("no se encontro la memoria [%d]\n",memoria->numero);
                }
                free(memoria);
            } else {
                printf("revisar los parametros\n");
            }
            break;
        }
        default: {
            printf("Comando no reconocido\n");
            log_error(file_log, "Comando no reconocido");
        }
    }

    if (flagErrorSintaxis) {
        printf("Verificar el comando ingresado\n");
        log_error(file_log, "Verificar el comando ingresado");
    }
}

void consola() {
    char *comando;
    printf("Ingrese comando LQL\n");
    comando = readline(">");
    string_trim(&comando);
    log_info(file_log, "[Console] Ingresando comando");
    while (strcmp(comando, "exit") != 0) {
        armarComando(comando);
        free(comando);
        comando = readline(">");
        log_info(file_log, "[Console] Ingresando comando");
        string_trim(&comando);
    }
    free(comando);
}
