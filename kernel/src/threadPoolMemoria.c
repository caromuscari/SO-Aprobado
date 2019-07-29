//
// Created by miguelchauca on 28/04/19.
//

#include "threadPoolMemoria.h"

t_list *poolMemoria;
extern config *configuracion;
t_log *file_log_poolMemoria;
pthread_mutex_t mutex;

void logStatusListaMemoria(){
    st_kernel_memoria * memoriaK;
    log_info(file_log_poolMemoria,"----Estado de Memoria---");
    for (int i = 0; i < poolMemoria->elements_count ; ++i) {
        memoriaK = list_get(poolMemoria,i);
        if(memoriaK->activo){
        	char * num = string_itoa(memoriaK->memoria->numero);
            log_info(file_log_poolMemoria,"Numero = [%s]",num);
            log_info(file_log_poolMemoria,"Puerto [%s]", memoriaK->memoria->puerto);
            log_info(file_log_poolMemoria,"IP [%s]",memoriaK->memoria->ip);
            free(num);
        }

    }
}

void destroyKernelMemoria(st_kernel_memoria * kernelMemoria){
    //clean tags
    list_iterate(kernelMemoria->tags, free);
    list_destroy(kernelMemoria->tags);
    //clean tipos
    list_iterate(kernelMemoria->tipos, free);
    list_destroy(kernelMemoria->tipos);
    //clean hitory
    list_iterate(kernelMemoria->history, free);
    list_destroy(kernelMemoria->history);
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

void destroyPoolMemory(t_list * listaMemoria){//revisar
    int i;
    st_kernel_memoria * kernelMemoria;
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
        newKernelMemoria->activo = kernelMemoria->activo;
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
        newKernelMemoria->history = list_create();
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

st_memoria * getMemoriaSHC(char *text) {
    int i;
    st_kernel_memoria *memoria;
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        memoria = list_get(poolMemoria, i);
        if (memoria->activo && existeTipoDeCriterio(memoria->tipos, StrongHashConsistency)) {
            if (existeTag(memoria->tags, text)) {
                memoria->count++;
                memoria->countSHC++;
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
        if (memoria->activo && existeTipoDeCriterio(memoria->tipos, StrongHashConsistency)) {
            if (flagFirst) {
                countMinimo = memoria->countSHC;
                posicion = i;
                flagFirst = false;
            } else {
                if (memoria->countSHC < countMinimo) {
                    countMinimo = memoria->countSHC;
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
        memoria->countSHC++;
        return clonarMemoria(memoria->memoria);
    }
    return NULL;
}

st_memoria * getMemoriaSC(char *text) {
    int i;
    st_kernel_memoria *memoria;
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        memoria = list_get(poolMemoria, i);
        if (memoria->activo && existeTipoDeCriterio(memoria->tipos, StrongConsistency)) {
            if (existeTag(memoria->tags, text)) {
                memoria->count++;
                memoria->countSC++;
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
        if (memoria->activo && existeTipoDeCriterio(memoria->tipos, StrongConsistency)) {
            if (flagFirst) {
                countMinimo = memoria->countSC;
                posicion = i;
                flagFirst = false;
            } else {
                if (memoria->count < countMinimo) {
                    countMinimo = memoria->countSC;
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
        memoria->countSC++;
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
                mini = memoria->countEC;
                flagFisrt = false;
            }else{
                if(memoria->count < mini){
                    pos = i;
                    mini = memoria->countEC;
                }
            }
        }
    }
    if(pos != -1){
        memoria = list_get(poolMemoria, pos);
        memoria->count++;
        memoria->countEC++;
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
            memoria = getMemoriaSC(text);
            break;
        }
        case StrongHashConsistency: {
            memoria = getMemoriaSHC(text);
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
    st_memoria *memoria = malloc(sizeof(st_memoria));

    memoria->numero = data->numero;
    memoria->puerto = strdup(data->puerto);
    memoria->ip = strdup(data->ip);

    k_memoria->tipos = list_create();
    k_memoria->activo = true;
    k_memoria->tags = list_create();
    k_memoria->history = list_create();
    k_memoria->memoria = memoria;
    k_memoria->count = 0;
    k_memoria->countEC = 0;
    k_memoria->countSC = 0;
    k_memoria->countSHC = 0;

    return k_memoria;
}

st_kernel_memoria *existeMemoria(t_list * listaMemoria, int numeroMemoria) {
    int i;
    st_kernel_memoria *memoria;
    for (i = 0; i < listaMemoria->elements_count; ++i) {//revisar
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
        kernelMemoria = list_remove(poolMemoria,i);
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
        log_info(file_log_poolMemoria, "no se encontro esa memoria");
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
    log_info(file_log_poolMemoria,"[gossiping] Actulizando lista de memorias");
    st_kernel_memoria *newKernelMemoria;
    st_memoria *stMemoria;
    pthread_mutex_lock(&mutex);
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
    }
    pthread_mutex_unlock(&mutex);
}

st_history_request * newHistory(enum OPERACION operacion, TypeCriterio criterio){
    st_history_request * history = malloc(sizeof(st_history_request));
    history->startTime = obtenerMilisegundosDeHoy();
    history->operacion = operacion;
    history->tipoCriterio = criterio;
    return history;
}

void addHistory(st_history_request * historyRequest, int numeroMemoria){
    log_info(file_log_poolMemoria,"[historyMemoria] agregando historial en memoria [%d]",numeroMemoria);
    pthread_mutex_lock(&mutex);
    st_kernel_memoria * kernelMemoria = existeMemoria(poolMemoria,numeroMemoria);
    if(kernelMemoria){
        list_add(kernelMemoria->history,historyRequest);
    }else{
        log_info(file_log_poolMemoria,"[historyMemoria] no se encontro la memoria %d", numeroMemoria);
    }
    pthread_mutex_unlock(&mutex);
}

t_list * getHistoryByRange(double start, double end){
    int i,j;
    st_kernel_memoria * kernelMemoria = NULL;
    st_metrica_memoria * metricaMemoria = NULL;
    st_history_request * historyRequest = NULL;
    t_list * listaMemoriaHistory = list_create();
    TypeCriterio * criterio = NULL;
    TypeCriterio * newCriterio = NULL;
    pthread_mutex_lock(&mutex);
    for (i = 0; i < poolMemoria->elements_count ; ++i) {
        kernelMemoria = list_get(poolMemoria, i);
        metricaMemoria = malloc(sizeof(st_metrica_memoria));
        metricaMemoria->numeroMemoria = kernelMemoria->memoria->numero;
        metricaMemoria->history = list_create();
        metricaMemoria->tipos = list_create();
        //copiar tipos
        for (j = 0; j < kernelMemoria->tipos->elements_count; ++j) {
            criterio = list_get(kernelMemoria->tipos,j);
            newCriterio = malloc(sizeof(TypeCriterio));
            *newCriterio = *criterio;
            list_add(metricaMemoria->tipos,newCriterio);
        }
        for (j = 0; j < kernelMemoria->history->elements_count ; ++j) {
            historyRequest = list_get(kernelMemoria->history,j);
            if(historyRequest->startTime >= start && historyRequest->endTime <= end){
                list_add(metricaMemoria->history,historyRequest);
            }
        }
        list_add(listaMemoriaHistory,metricaMemoria);
    }
    pthread_mutex_unlock(&mutex);
    return listaMemoriaHistory;
}

int journalMemoria(st_memoria * memoria){
    void * buffer = strdup("1");
    st_messageResponse * respuestPeticion = consultarAMemoria(memoria->ip,memoria->puerto,JOURNAL,buffer,1);
    if(respuestPeticion){
        int resultado = NO_SALIO_OK;
        switch (respuestPeticion->cabezera.codigo){
            case SUCCESS:{
                printf("se realizo correctamente el journal\n");
                resultado = SALIO_OK;
                break;
            }
            case NOSUCCESS:{
                printf("no se puedo relizar el journal\n");
                break;
            }
            default:{
                printf("no entiendo el codigo de respuesta\n");
                break;
            }
        }
        free(buffer);
        destroyStMessageResponse(respuestPeticion);
        return resultado;
    }else{
        free(buffer);
        log_error(file_log_poolMemoria,"[Journal] No hubo respuesta en el Journal");
        return SE_DESCONECTO_SOCKET;
    }

}

void hacerJournal(){
    int i;
    t_list * listaMemoria = clonarPool();//revisar
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

void *devolverListaMemoria(size_t *size_paquetes) {
    st_data_memoria *memoria = malloc(sizeof(st_data_memoria));
    memoria->numero = -1;
    memoria->listaMemorias = list_create();
    pthread_mutex_lock(&mutex);
    void * buffer = serealizarMemoria(memoria, size_paquetes);
    pthread_mutex_unlock(&mutex);
    list_destroy(memoria->listaMemorias);
    free(memoria);
    return buffer;
}

void *loadPoolMemori() {
    //conectarse con la memoria y pelirle la lista
    poolMemoria = list_create();
    file_log_poolMemoria = crear_archivo_log("PoolMemoria", false, "./PoolMemoria.log");
    st_messageResponse * respuestaMesanje = NULL;
    void *buffer = NULL;
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("\n mutex init failed\n");
        pthread_exit(NULL);
    }
    while (1) {
        size_t  size;
        buffer = devolverListaMemoria(&size);
        log_info(file_log_poolMemoria,"[gossiping] Haciendo Gossiping");
        respuestaMesanje = consultarAMemoria(configuracion->IP_MEMORIA, configuracion->PUERTO_MEMORIA,BUSCARTABLAGOSSIPING,buffer,size);
        if(respuestaMesanje){
            switch (respuestaMesanje->cabezera.codigo){
                case SUCCESS:{
                	st_data_memoria * mem = deserealizarMemoria(respuestaMesanje->buffer, respuestaMesanje->cabezera.sizeData);
                	updateListaMemorias(mem);
                    destroyListaDataMemoria(mem);
                	destroyStMessageResponse(respuestaMesanje);
                    break;
                }
                default:{
                    log_info(file_log_poolMemoria,"[gossiping] no entiendo el codigo re respuesta\n");
                    break;
                }
            }
        }else{
            log_error(file_log_poolMemoria,"[gossiping] No hubo respuesta en el gossiping");
        }
        free(buffer);
        logStatusListaMemoria();
        sleep(configuracion->REFRESH_GOSSIPING / 1000);
    }
}
