//
// Created by miguelchauca on 04/05/19.
//

#include "threadPlanificador.h"

t_list *listaDeScript;
sem_t colaDeListos;
sem_t procesadores;
pthread_mutex_t mutex;
extern config *configuracion;
extern t_log *file_log;

st_instruccion *crearInstruccion(void *instruccion, enum OPERACION type) {
    st_instruccion *newInstruccion = malloc(sizeof(st_instruccion));
    newInstruccion->operacion = type;
    newInstruccion->instruccion = instruccion;
    return newInstruccion;
}

st_script *crearNuevoScript(char *id, t_list *listaInstrucciones) {
    st_script *stScript = malloc(sizeof(st_script));
    stScript->id = strdup(id);
    stScript->listaDeInstrucciones = listaInstrucciones;
    return stScript;
}

void destroyInctruccion(st_instruccion *instruccion) {
    switch (instruccion->operacion) {
        case INSERT: {
            destroyInsert(instruccion->instruccion);
            break;
        }
        case SELECT: {
            destoySelect(instruccion->instruccion);
            break;
        }
        case CREATE: {
            destroyCreate(instruccion->instruccion);
            break;
        }
        case DROP: {
            destroyDrop(instruccion->instruccion);
            break;
        }
        case DESCRIBE: {
            destroyDescribe(instruccion->instruccion);
            break;
        }
    }
    free(instruccion);
}

void destroyListaInstruciones(t_list *instrucciones) {
    st_instruccion *instruccionScript;
    int i;
    for (i = 0; i < instrucciones->elements_count; ++i) {
        instruccionScript = list_remove(instrucciones, i);
        destroyInctruccion(instruccionScript);
    }
    list_destroy(instrucciones);
}

void destroyScript(st_script *stScript) {
    destroyListaInstruciones(stScript->listaDeInstrucciones);
    free(stScript->id);
    free(stScript);
}

t_list *tomarInstrucciones(t_list *listaInstrucciones) {
    if (listaInstrucciones->elements_count > configuracion->QUANTUM) {
        return list_take_and_remove(listaInstrucciones, configuracion->QUANTUM);
    } else {
        return list_take_and_remove(listaInstrucciones, listaInstrucciones->elements_count);
    }
}

st_script *tomarScript() {
    st_script *script = NULL;
    pthread_mutex_lock(&mutex);
    if (listaDeScript->elements_count > 0) {
        script = list_remove(listaDeScript, 0);
    }
    pthread_mutex_unlock(&mutex);
    return script;
}

void cargarNuevoScript(st_script *newScript) {
    pthread_mutex_lock(&mutex);
    list_add(listaDeScript, newScript);
    pthread_mutex_unlock(&mutex);
    sem_post(&colaDeListos);
}

void ejecutarScript() {
    int i, resultado = NO_SALIO_OK;
    st_instruccion *instruccionScript = NULL;//Hay que liberar?
    st_memoria *datomemoria = NULL;
    st_script *script = tomarScript();
    char *tag;
    bool hayError;
    TypeCriterio typeCriterio;
    st_history_request * historyRequest = NULL;
    printf("\n[Planificador] Ejecutando Script [%s]\n", script->id);
    t_list *instrucciones = tomarInstrucciones(script->listaDeInstrucciones);
    //ejecutamos instruciones tomadas
    for (i = 0; i < instrucciones->elements_count; ++i) {
        hayError = false;
        historyRequest = NULL;
        instruccionScript = list_get(instrucciones, i);
        typeCriterio = getCriterioBYInstruccion(instruccionScript->instruccion, instruccionScript->operacion);
        if (typeCriterio == NO_SE_ENCONTRO_TABLA) {
            hayError = true;
            printf("[Planificador] No se encontro tabla\n");
            log_error(file_log,"[Planificador] No se encontro la tabla");
        }else {
            tag = generarTag(typeCriterio, instruccionScript->instruccion, instruccionScript->operacion);
            datomemoria = getMemoria(typeCriterio, tag);
            if (datomemoria) {
                if(instruccionScript->operacion == INSERT || instruccionScript->operacion == SELECT){
                    historyRequest = newHistory(instruccionScript->operacion,typeCriterio);
                }
                resultado = enviarRequestMemoria(instruccionScript, datomemoria);
                if (resultado == NO_SALIO_OK || resultado == SE_DESCONECTO_SOCKET) {
                    hayError = true;
                }
                if(historyRequest && resultado == SALIO_OK){
                    historyRequest->endTime = obtenerMilisegundosDeHoy() + configuracion->SLEEP_EJECUCION;
                    addHistory(historyRequest,datomemoria->numero);
                }
            } else {
                printf("[Planificador] No hay memoria disponible\n");
                resultado = MEMORIA_NO_DISPONIBLE;
                hayError = true;
            }
            free(tag);
        }
        if(hayError){
            log_error(file_log,"[Planificador] Hubo un error en la Ejecuccion se cancela el script");
            if(historyRequest){
                free(historyRequest);
            }
            break;
        }
        sleep(configuracion->SLEEP_EJECUCION / 1000);
    }
    printf("[Planificador] Fin Ejecucion de Script [%s]\n", script->id);
    //Evaluar resultado de Ejecucion
    if (resultado == SALIO_OK || resultado == NO_HAY_RESULTADO_EN_SELECT) {
        if (script->listaDeInstrucciones->elements_count > 0) {
            log_info(file_log,"[Planificador] agregando de nuevo el script");
            cargarNuevoScript(script);
        }else{
            log_info(file_log,"[Planificador] Se finalizo todo el script");
            destroyScript(script);
        }
    } else {
        log_info(file_log,"[Planificador] fallo la ejecuccion  [%d]",resultado);
        destroyScript(script);
    }
    destroyListaInstruciones(instrucciones);
}

void atenderScriptEntrantes() {
    pthread_t pthread_execution;//Revisar
    while (1) {
        sem_wait(&colaDeListos);
        sem_wait(&procesadores);
        pthread_create(&pthread_execution, NULL, (void*)ejecutarScript, NULL);
        pthread_detach(pthread_execution);
        sem_post(&procesadores);
    }
}

void inicialPlanificador() {
    listaDeScript = list_create();
    sem_init(&colaDeListos, 0, 0);
    sem_init(&procesadores, 0, configuracion->MULTIPROCESAMIENTO);
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("\n mutex init failed\n");
        pthread_exit(NULL);
    }
    atenderScriptEntrantes();
}
