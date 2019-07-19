//
// Created by miguelchauca on 28/04/19.
//

#include "threadPoolMemoria.h"

t_list *poolMemoria;
extern config *configuracion;
extern t_log *file_log;
pthread_mutex_t mutex;

void showMemoria(){
    st_kernel_memoria * memoriaK;
    printf("-----------------------------------------------\n");
    for (int i = 0; i < poolMemoria->elements_count ; ++i) {
        memoriaK = list_get(poolMemoria,i);
        if(memoriaK->activo){
            printf("ip [%s]\n",memoriaK->memoria->ip);
            printf("puerto [%s]\n",memoriaK->memoria->puerto);
            printf("numero [%d]\n",memoriaK->memoria->numero);
        }

    }
}

void destroyKernelMemoria(st_kernel_memoria * kernelMemoria){
    //clean tags
    list_iterate(kernelMemoria->tags, free);
    //clean tipos
    list_iterate(kernelMemoria->tipos, free);
    destroyMemoria(kernelMemoria->memoria);
    free(kernelMemoria);
}

st_memoria * clonarMemoria(st_memoria * memoriaAclonar){
    st_memoria * memoria = malloc(sizeof(st_memoria));
    memoria->numero = memoriaAclonar->numero;
    memoria->puerto = strdup(memoriaAclonar->puerto);
    memoria->ip = strdup(memoriaAclonar->ip);
    return memoria;
}

void destroyPoolMemory(t_list * listaMemoria){
    int i;
    st_kernel_memoria * kernelMemoria;
    char * tag;
    for (i = 0; i < listaMemoria->elements_count ; ++i) {
        kernelMemoria = list_get(listaMemoria,i);
        destroyKernelMemoria(kernelMemoria);
    }
}

t_list * clonarPool(){
    t_list * poolClone =  list_create();
    int i,j;
    st_kernel_memoria * kernelMemoria;
    st_kernel_memoria * newKernelMemoria;
    char * tag;
    TypeCriterio * criterio;
    TypeCriterio * newCriterio;
    pthread_mutex_lock(&mutex);
    for (i = 0; i <  poolMemoria->elements_count; ++i) {
        kernelMemoria = list_get(poolMemoria,i);
        newKernelMemoria = malloc(sizeof(st_kernel_memoria));
        newKernelMemoria->count = kernelMemoria->count;
        newKernelMemoria->activo = newKernelMemoria->activo;
        //copy memoria
        newKernelMemoria->memoria = malloc(sizeof(st_memoria));
        newKernelMemoria->memoria->numero = kernelMemoria->memoria->numero;
        newKernelMemoria->memoria->puerto = strdup(kernelMemoria->memoria->puerto);
        newKernelMemoria->memoria->ip = strdup(kernelMemoria->memoria->ip);
        //copy tags
        newKernelMemoria->tags = list_create();
        for (j = 0; j < newKernelMemoria->tags->elements_count; ++j) {
            tag = list_get(newKernelMemoria->tags,j);
            list_add(newKernelMemoria->tags,strdup(tag));
        }
        //copy criterios
        newKernelMemoria->tipos = list_create();
        for (j = 0; j < newKernelMemoria->tipos->elements_count; ++j) {
            criterio = list_get(newKernelMemoria->tipos,j);
            newCriterio = malloc(sizeof(TypeCriterio));
            *newCriterio = *criterio;
            list_add(newKernelMemoria->tags,newCriterio);
        }
        //add item a la lista clonada
        list_add(poolClone,newKernelMemoria);
    }
    pthread_mutex_unlock(&mutex);
    return  poolClone;
}

bool existeTipoDeCriterio(t_list *criterios, TypeCriterio tipo) {
    int i;
    TypeCriterio *criterio;
    for (i = 0; i < criterios->elements_count; ++i) {
        criterio = list_get(criterios, i);
        if (*criterio == tipo) {
            return true;
        }
    }
    return false;
}

bool existeTag(t_list *tags, char *text) {
    int i;
    char *tag;
    for (i = 0; i < tags->elements_count; ++i) {
        tag = list_get(tags, i);
        if (strcmp(tag, text) == 0) {
            return true;
        }
    }
    return false;
}

st_memoria * getMemoriaSCHSC(char *text, TypeCriterio tipo) {
    int i;
    st_kernel_memoria *memoria;
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        memoria = list_get(poolMemoria, i);
        if (memoria->activo && existeTipoDeCriterio(memoria->tipos, tipo)) {
            if (existeTag(memoria->tags, text)) {
                memoria->count++;
                return clonarMemoria(memoria->memoria);
            }
        }
    }
    //buscar que memoria esta disponible tomamos al primero como posible minimo
    int posicion = -1;
    int countMinimo = 0;
    bool flagFirst = true;
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        memoria = list_get(poolMemoria, i);
        if (memoria->activo && existeTipoDeCriterio(memoria->tipos, tipo)) {
            if (flagFirst) {
                countMinimo = memoria->count;
                posicion = i;
                flagFirst = false;
            } else {
                if (memoria->count < countMinimo) {
                    countMinimo = memoria->count;
                    posicion = i;
                }
            }
        }
    }
    //cargamos un nuevo tag a la memoria elegida
    if(posicion != -1){
        memoria = list_get(poolMemoria, posicion);
        list_add(memoria->tags,strdup(text));
        memoria->count++;
        return clonarMemoria(memoria->memoria);
    }
    return NULL;
}

st_memoria * getMemoriaEventual(){
    int i;
    st_kernel_memoria *memoria;
    int pos = -1;
    int mini = 0;
    bool flagFisrt = true;
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        memoria = list_get(poolMemoria, i);
        if (memoria->activo && existeTipoDeCriterio(memoria->tipos, EventualConsistency)) {
            if(flagFisrt){
                pos = i;
                mini = memoria->count;
                flagFisrt = false;
            }else{
                if(memoria->count < mini){
                    pos = i;
                    mini = memoria->count;
                }
            }
        }
    }
    if(pos != -1){
        memoria = list_get(poolMemoria, pos);
        memoria->count++;
        return clonarMemoria(memoria->memoria);
    }

    return NULL;
}

st_memoria * getMemoriaRandom(){
    int i;
    st_kernel_memoria *memoria;
    int pos = -1;
    int mini = 0;
    bool flagFisrt = true;
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        memoria = list_get(poolMemoria, i);
        if(flagFisrt){
            pos = i;
            mini = memoria->count;
            flagFisrt = false;
        }else{
            if(memoria->count < mini){
                pos = i;
                mini = memoria->count;
            }
        }
    }
    if(pos != -1){
        memoria = list_get(poolMemoria, pos);
        memoria->count++;
        return clonarMemoria(memoria->memoria);
    }

    return NULL;
}

st_memoria *getMemoria(TypeCriterio tipo, char *text) {
    st_memoria * memoria;
    pthread_mutex_lock(&mutex);
    switch (tipo) {
        case StrongConsistency: {
            memoria = getMemoriaSCHSC(text, StrongConsistency);
            break;
        }
        case StrongHashConsistency: {
            memoria = getMemoriaSCHSC(text, StrongHashConsistency);
            break;
        }
        case EventualConsistency: {
            memoria = getMemoriaEventual();
            break;
        }
        default: {
            memoria = getMemoriaRandom();
        }
    }
    pthread_mutex_unlock(&mutex);
    return memoria;
}

st_kernel_memoria *cargarNuevaKernelMemoria(st_memoria *data) {
    st_kernel_memoria *k_memoria = malloc(sizeof(st_kernel_memoria));
    st_memoria *memoria = malloc(sizeof(st_data_memoria));

    memoria->numero = data->numero;
    memoria->puerto = strdup(data->puerto);
    memoria->ip = strdup(data->ip);

    k_memoria->tipos = list_create();
    k_memoria->activo = true;
    k_memoria->tags = list_create();
    k_memoria->memoria = memoria;
    k_memoria->count = 0;

    return k_memoria;
}

st_kernel_memoria *existeMemoria(t_list * listaMemoria, int numeroMemoria) {
    int i;
    st_kernel_memoria *memoria;
    for (i = 0; i < listaMemoria->elements_count; ++i) {
        memoria = list_get(listaMemoria, i);
        if (memoria->memoria->numero == numeroMemoria) {
            return memoria;
        }
    }
    return NULL;
}

void eliminarMemoria(int numberMemoria){
    int i;
    st_kernel_memoria * kernelMemoria = NULL;
    printf("eliminando memoria [%d]\n",numberMemoria);
    pthread_mutex_lock(&mutex);
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        kernelMemoria = list_get(poolMemoria,i);
        if(kernelMemoria->memoria->numero == numberMemoria){
            break;
        }
    }
    if(kernelMemoria){
        list_remove(poolMemoria,i);
        destroyKernelMemoria(kernelMemoria);
    }
    pthread_mutex_unlock(&mutex);
}

bool setTipoConsistencia(int number, TypeCriterio tipo) {
    bool flagSolucion = false;
    TypeCriterio *auxTipo;
    pthread_mutex_lock(&mutex);
    st_kernel_memoria *memoria = existeMemoria(poolMemoria, number);
    if (memoria && memoria->activo) {
        flagSolucion = true;
        auxTipo = malloc(sizeof(TypeCriterio));
        *auxTipo = tipo;
        list_add(memoria->tipos, auxTipo);
    } else {
        log_info(file_log, "no se encontro esa memoria");
    }
    pthread_mutex_unlock(&mutex);
    return flagSolucion;
}

void agregarMemoria(st_kernel_memoria *kernelMemoria) {
    list_add(poolMemoria, kernelMemoria);
}

void updateMemoria(st_kernel_memoria *kernelMemoria, st_memoria *stMemoria) {
    destroyMemoria(kernelMemoria->memoria);
    st_memoria *memoria = malloc(sizeof(st_data_memoria));
    memoria->numero = stMemoria->numero;
    memoria->puerto = strdup(stMemoria->puerto);
    memoria->ip = strdup(stMemoria->ip);
    kernelMemoria->memoria = memoria;
    kernelMemoria->activo = true;
}

void updateListaMemorias(st_data_memoria *dataMemoria) {
    //crear la memoria que consulto
    st_kernel_memoria *newKernelMemoria;
    st_memoria *stMemoria;
    pthread_mutex_lock(&mutex);
    newKernelMemoria = existeMemoria(poolMemoria,dataMemoria->numero);
    if (newKernelMemoria == NULL) {
        //agregar
        stMemoria = malloc(sizeof(st_memoria));
        stMemoria->numero = dataMemoria->numero;
        stMemoria->puerto = strdup(configuracion->PUERTO_MEMORIA);
        stMemoria->ip = strdup(configuracion->IP_MEMORIA);
        newKernelMemoria = cargarNuevaKernelMemoria(stMemoria);
        agregarMemoria(newKernelMemoria);
        destroyMemoria(stMemoria);
    } else {
        //update
        newKernelMemoria->memoria->numero = dataMemoria->numero;
        newKernelMemoria->activo = true;
    }
    int i;
    for (i = 0; i < dataMemoria->listaMemorias->elements_count; ++i) {
        stMemoria = list_get(dataMemoria->listaMemorias, i);
        newKernelMemoria = existeMemoria(poolMemoria,stMemoria->numero);
        if (newKernelMemoria == NULL) {
            //adding
            newKernelMemoria = cargarNuevaKernelMemoria(stMemoria);
            agregarMemoria(newKernelMemoria);
        } else {
            //update
            updateMemoria(newKernelMemoria, stMemoria);
        }
        destroyMemoria(stMemoria);
    }
    pthread_mutex_unlock(&mutex);
}

void CleanListaMemoria() {
    int i;
    st_kernel_memoria *kernelMemoria;
    pthread_mutex_lock(&mutex);
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        kernelMemoria = list_get(poolMemoria, i);
        kernelMemoria->activo = false;
    }
    pthread_mutex_unlock(&mutex);
}

int journalMemoria(st_memoria * memoria){
    void * buffer = strdup("1");
    st_messageResponse * respuestPeticion = consultarAMemoria(memoria->ip,memoria->puerto,JOURNAL,buffer,1);
    if(respuestPeticion){
        int resultado = NO_SALIO_OK;
        switch (respuestPeticion->cabezera.codigo){
            case SUCCESS:{
                printf("se realizo correctamente el journal");
                resultado = SALIO_OK;
                break;
            }
            case NOSUCCESS:{
                printf("no se puedo relizar el journal");
                break;
            }
            default:{
                printf("no entiendo el codigo de respuesta");
                break;
            }
        }
        free(buffer);
        destroyStMessageResponse(respuestPeticion);
        return resultado;
    }else{
        free(buffer);
        log_error(file_log,"[Journal] No hubo respuesta en el Journal");
        return SE_DESCONECTO_SOCKET;
    }

}

void hacerJournal(){
    int i;
    t_list * listaMemoria = clonarPool();
    st_kernel_memoria * kernelMemoria;
    for (i = 0; i < listaMemoria->elements_count ; ++i) {
        kernelMemoria = list_get(listaMemoria,i);
        printf("Journal de memoria [%d]\n",kernelMemoria->memoria->numero);
        if(journalMemoria(kernelMemoria->memoria) == SE_DESCONECTO_SOCKET){
            eliminarMemoria(kernelMemoria->memoria->numero);
        }
    }
    destroyPoolMemory(listaMemoria);
}

void *loadPoolMemori() {
    //conectarse con la memoria y pelirle la lista
    poolMemoria = list_create();
    st_messageResponse * respuestaMesanje = NULL;
    void *buffer = NULL;
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("\n mutex init failed\n");
        pthread_exit(NULL);
    }
    while (1) {
        CleanListaMemoria();
        buffer = strdup("1");
        respuestaMesanje = consultarAMemoria(configuracion->IP_MEMORIA, configuracion->PUERTO_MEMORIA,BUSCARTABLAGOSSIPING,buffer,1);
        if(respuestaMesanje){
            switch (respuestaMesanje->cabezera.codigo){
                case SUCCESS:{
                    updateListaMemorias(deserealizarMemoria(respuestaMesanje->buffer, respuestaMesanje->cabezera.sizeData));
                    destroyStMessageResponse(respuestaMesanje);
                    break;
                }
                default:{
                    log_info(file_log,"[gossiping] no entiendo el codigo re respuesta\n");
                    break;
                }
            }
        }else{
            log_error(file_log,"[gossiping] No hubo respuesta en el gossiping");
        }
        free(buffer);
        sleep(10);
    }
}