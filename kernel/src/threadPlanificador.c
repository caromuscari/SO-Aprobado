//
// Created by miguelchauca on 04/05/19.
//

#include "threadPlanificador.h"

t_list *listaDeScript;
sem_t colaDeListos;
sem_t procesadores;
pthread_mutex_t mutex;
extern t_log *file_log;
extern config *configuracion;

stinstruccion *crearInstruccion(void *instruccion, enum OPERACION type, TypeCriterio criterio) {
    stinstruccion *newInstruccion = malloc(sizeof(stinstruccion));
    newInstruccion->operacion = type;
    newInstruccion->instruccion = instruccion;
    newInstruccion->criteio = criterio;
    newInstruccion->tag = generarTag(criterio, instruccion, type);
    return newInstruccion;
}

st_script *crearNuevoScript(char *id, t_list *listaInstrucciones) {
    st_script *stScript = malloc(sizeof(st_script));
    stScript->id = strdup(id);
    stScript->listaDeInstrucciones = listaInstrucciones;
}

void destroyInctruccion(stinstruccion *instruccion) {
    if (instruccion->tag) {
        free(instruccion->tag);
    }
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
    stinstruccion *instruccionScript;
    int i;
    for (i = 0; i < instrucciones->elements_count; ++i) {
        instruccionScript = list_get(instrucciones, i);
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

void * ejecutarScript(){
    int i, resultado = NO_SALIO_OK;
    stinstruccion *instruccionScript;
    st_memoria *datomemoria;
    st_script *script = tomarScript();
    printf("Ejecutando Script ----- [%s]\n", script->id);
    t_list *instrucciones = tomarInstrucciones(script->listaDeInstrucciones);
    //ejecutamos instruciones tomadas
    for (i = 0; i < instrucciones->elements_count; ++i) {
        instruccionScript = list_get(instrucciones, i);
        datomemoria = getMemoria(instruccionScript->criteio, instruccionScript->tag);
        if (datomemoria) {
            resultado = enviarRequestMemoria(instruccionScript, datomemoria);
            if (resultado == NO_SALIO_OK || resultado == SE_DESCONECTO_SOCKET) {
                break;
            }
        } else {
            printf("no hay memoria disponible\n");
            resultado = MEMORIA_NO_DISPONIBLE;
            break;
        }
    }
    printf("Fin Ejecucion de Script ----- [%s]\n", script->id);
    //Evaluar resultado de Ejecucion
    if (resultado == SALIO_OK) {
        cargarNuevoScript(script);
    } else {
        destroyScript(script);
    }
    destroyListaInstruciones(instrucciones);
}

void atenderScriptEntrantes() {
    pthread_t pthread_execution;
    while (1) {
        sem_wait(&colaDeListos);
        sem_wait(&procesadores);
        pthread_create(&pthread_execution,NULL, ejecutarScript, NULL);
        pthread_detach(pthread_execution);
        ejecutarScript();
        sleep(configuracion->SLEEP_EJECUCION);
        sem_post(&procesadores);
    }
}

void *inicialPlanificador() {
    listaDeScript = list_create();
    sem_init(&colaDeListos, 0, 0);
    sem_init(&procesadores, 0, configuracion->MULTIPROCESAMIENTO);
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("\n mutex init failed\n");
        pthread_exit(NULL);
    }
    atenderScriptEntrantes();
}
