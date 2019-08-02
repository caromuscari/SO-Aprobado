//
// Created by miguelchauca on 13/07/19.
//

#include "gossiping.h"

t_list * seedFallidas;
pthread_mutex_t mutex;
t_list *listaTablas;
extern t_log *file_log;
extern t_configuracionMemoria *configMemoria;

t_list * clonadoListad(){
    t_list * listaClonada = list_create();
    int i;
    st_memoria * memoria = NULL;
    st_memoria * cloneMemoria = NULL;
    pthread_mutex_lock(&mutex);
    for (i = 0; i < listaTablas->elements_count; i++) {
        memoria = list_get(listaTablas,i);
        cloneMemoria = malloc(sizeof(st_memoria));
        cloneMemoria->numero = memoria->numero;
        cloneMemoria->puerto = strdup(memoria->puerto);
        cloneMemoria->ip = strdup(memoria->ip);
        list_add(listaClonada,cloneMemoria);
    }
    pthread_mutex_unlock(&mutex);
    return listaClonada;
}

bool existeMemoria(int numeroMemoria) {
    int i;
    st_memoria *memoria;
    pthread_mutex_lock(&mutex);
    for (i = 0; i < listaTablas->elements_count; ++i) {
        memoria = list_get(listaTablas, i);
        if (memoria->numero == numeroMemoria) {
        	pthread_mutex_unlock(&mutex);
        	return true;
        }
    }
    pthread_mutex_unlock(&mutex);
    return false;
}

void logStatusListaMemoria(t_list * lista){
    st_memoria *memoria;
    int i;
    log_info(file_log,"----Estado de Memoria---");
    pthread_mutex_lock(&mutex);
    for (i = 0; i < lista->elements_count ; ++i) {
        memoria = list_get(lista, i);
        log_info(file_log, "Numero = [%d]", memoria->numero);
        log_info(file_log, "Puerto [%s]", memoria->puerto);
        log_info(file_log, "IP [%s]", memoria->ip);
    }
    pthread_mutex_unlock(&mutex);
}

void addSeedFallidas(char *ip,char * puerto){
    st_memoria * memoria = malloc(sizeof(st_memoria));
    memoria->numero  = 0;
    memoria->ip = strdup(ip);
    memoria->puerto = strdup(puerto);
    list_add(seedFallidas,memoria);
}

void addNewMemoria(st_memoria * memoria){
    log_info(file_log, "[gossiping] Agregando Memoria [%d]",memoria->numero);
    pthread_mutex_lock(&mutex);
    list_add(listaTablas, memoria);
    pthread_mutex_unlock(&mutex);
}

void removeMemoriaFallida(st_memoria * memoria){
    int i;
    bool encontroAlgo = false;
    st_memoria * auxMemoria = NULL;
    log_info(file_log, "[gossiping] removiendo memoria  IP=[%s] puerto=[%s]",memoria->ip,memoria->puerto);
    pthread_mutex_lock(&mutex);
    for (i = 0; i < listaTablas->elements_count ; ++i) {
        auxMemoria = list_get(listaTablas,i);
        if(strcmp(memoria->ip,auxMemoria->ip) == 0 && strcmp(memoria->puerto,auxMemoria->puerto) == 0){
            encontroAlgo = true;
            break;
        }
    }
    if(encontroAlgo){
        auxMemoria = list_remove(listaTablas,i);
    }
    pthread_mutex_unlock(&mutex);
    if(encontroAlgo && auxMemoria){
        destroyMemoria(auxMemoria);
    }
}

void *devolverListaMemoria(size_t *size_paquetes) {
    st_data_memoria *memoria = malloc(sizeof(st_data_memoria));
    if(!existeMemoria(configMemoria->NRO_MEMORIA)){
        st_memoria * stMemoria = malloc(sizeof(st_memoria));
        stMemoria->numero = configMemoria->NRO_MEMORIA;
        stMemoria->ip = strdup(configMemoria->IP_MEMORIA);
        stMemoria->puerto = strdup(configMemoria->PUERTO);
        addNewMemoria(stMemoria);
    }
    memoria->numero = configMemoria->NRO_MEMORIA;
    memoria->listaMemorias = listaTablas;
    pthread_mutex_lock(&mutex);
    void * buffer = serealizarMemoria(memoria, size_paquetes);
    pthread_mutex_unlock(&mutex);
    free(memoria);
    return buffer;
}

void actualizarListaMemorias(st_data_memoria * dataMemoria){
    int i;
    st_memoria *AuxMemoria = NULL;
    log_info(file_log, "[gossiping] Iterando lista de memoria [%d]",dataMemoria->numero);
    for (i = 0; i < dataMemoria->listaMemorias->elements_count; ++i) {
        AuxMemoria = list_get(dataMemoria->listaMemorias, i);
        if (!existeMemoria(AuxMemoria->numero)) {
            addNewMemoria(AuxMemoria);
        } else {
            destroyMemoria(AuxMemoria);
        }
    }
}

void consultarEstadoMemoria(char *ip, char *puerto) {
    int control = 0;
    st_data_memoria *dataMemoria;
    int fdClient = establecerConexion(ip, puerto, file_log, &control);
    if (control != 0) {
        log_error(file_log, "[gossiping] No se pudo establecer conexion la memoria IP=[%s] puerto=[%s]",ip,puerto);
        addSeedFallidas(ip,puerto);
        return;
    } else {
        header request;
        size_t size = 0;
        void * buffer = devolverListaMemoria(&size);
        request.letra = 'M';
        request.codigo = BUSCARTABLAGOSSIPING;
        request.sizeData = size;
        message * paquete = createMessage(&request,buffer);
        enviar_message(fdClient, paquete, file_log, &control);
        free(buffer);
        free(paquete->buffer);
        free(paquete);
        if (control != 0) {
            return;
        } else {
            control = 0;
            header response;
            buffer = getMessage(fdClient, &response, &control);
            if (buffer == NULL) {
                log_error(file_log, "[gossiping] No se Desconecto la memoria IP=[%s] puerto=[%s]",ip,puerto);
                return;
            } else {
                log_info(file_log, "[gossiping] Evaluando Respuesta IP=[%s] puerto=[%s]",ip,puerto);
                dataMemoria = deserealizarMemoria(buffer, response.sizeData);
                actualizarListaMemorias(dataMemoria);
                list_destroy(dataMemoria->listaMemorias);
                free(dataMemoria);
                free(buffer);
            }
        }

    }
}

void cleanListaMemorias(t_list * lista){
    int i;
    st_memoria * memoria;
    for (i = 0; i < lista->elements_count; ++i) {
        memoria = list_get(lista,i);
        destroyMemoria(memoria);
    }
    list_clean(lista);
}

void *pthreadGossping() {
    listaTablas = list_create();
    seedFallidas = list_create();
    st_memoria * memoria;
    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        pthread_exit(NULL);
    }
    int i;
    while (true) {
        log_info(file_log, "[gossiping] Iniciando --> busqueda de data de otras memorias");
        for (i = 0; i < configMemoria->IP_SEEDS->elements_count; ++i) {
            consultarEstadoMemoria(list_get(configMemoria->IP_SEEDS, i), list_get(configMemoria->PUERTO_SEEDS, i));
        }
        //clonar lista para no bloquear
        t_list * listaCloneTabla = clonadoListad();
        st_memoria * auxMemoria = NULL;
        log_info(file_log, "[gossiping] Verificando status de tablas");
        for (i = 0; i < listaCloneTabla->elements_count; ++i) {
            auxMemoria = list_get(listaCloneTabla,i);
            if(auxMemoria->numero != configMemoria->NRO_MEMORIA){
                consultarEstadoMemoria(auxMemoria->ip,auxMemoria->puerto);
            }
        }
        //verificar seeed ingresado
        log_info(file_log, "[gossiping] Elimiando memorias fallidas");
        for (i = 0; i < seedFallidas->elements_count ; ++i) {
            memoria = list_get(seedFallidas,i);
            removeMemoriaFallida(memoria);
        }


        log_info(file_log, "[gossiping] Limpiando lista de seed fallidas");
        cleanListaMemorias(seedFallidas);
        logStatusListaMemoria(listaTablas);
        log_info(file_log, "[gossiping] Finalizando --> busqueda de data de otras memorias");
        sleep(configMemoria->TIEMPO_GOSSIPING/1000);
        log_info(file_log, "[gossiping] Limpiando memorias");
        cleanListaMemorias(listaCloneTabla);
        list_destroy(listaCloneTabla);
    }
    pthread_exit(NULL);
}
