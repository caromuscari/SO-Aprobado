//
// Created by miguelchauca on 13/07/19.
//

#include "gossiping.h"

t_list * seedFallidas;
pthread_mutex_t mutex;
t_list *listaTablas;
extern t_log *file_log;
extern t_configuracionMemoria *configMemoria;
extern pthread_mutex_t mutex, mutexSeeds;

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

void showMiguel(t_list *lista) {
    st_memoria *memoria;
    for (int i = 0; i < lista->elements_count; ++i) {
        memoria = list_get(lista, i);
        printf("%s\n", memoria->puerto);
        printf("%s\n", memoria->ip);
        printf("%d\n", memoria->numero);
    }
}

void addSeedFallidas(char *ip,char * puerto){
    st_memoria * memoria = malloc(sizeof(st_memoria));
    memoria->numero  = 0;
    memoria->ip = strdup(ip);
    memoria->puerto = strdup(puerto);
    pthread_mutex_lock(&mutexSeeds);
    list_add(seedFallidas,memoria);
    pthread_mutex_unlock(&mutexSeeds);
}

void cleanSeed(){
    int i;
    st_memoria * memoria;
    pthread_mutex_lock(&mutexSeeds);
    for (i = 0; i < seedFallidas->elements_count ; ++i) {
        memoria = list_remove(seedFallidas,i);
        destroyMemoria(memoria);
    }
    list_clean(seedFallidas);
    pthread_mutex_unlock(&mutexSeeds);
}

void addNewMemoria(st_memoria * memoria){
    pthread_mutex_lock(&mutex);
    list_add(listaTablas, memoria);
    pthread_mutex_unlock(&mutex);
}

void consultarEstadoMemoria(char *ip, char *puerto) {
    int control = 0;
    int i;
    st_data_memoria *dataMemoria;
    st_memoria *auxMemoria;
    int fdClient = establecerConexion(ip, puerto, file_log, &control);
    if (control != 0) {
        log_error(file_log, "No se pudo establecer conexion");
        addSeedFallidas(ip,puerto);
        return;
    } else {
        header request;
        request.letra = 'M';
        request.codigo = BUSCARTABLAGOSSIPING;
        request.sizeData = 1;
        void * buffer = strdup("1");
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
                return;
            } else {
                dataMemoria = deserealizarMemoria(buffer, response.sizeData);
                st_memoria *nuevoMemoria = malloc(sizeof(st_memoria));
                nuevoMemoria->numero = dataMemoria->numero;
                nuevoMemoria->ip = strdup(ip);
                nuevoMemoria->puerto = strdup(puerto);
                if (!existeMemoria(nuevoMemoria->numero)) {
                    addNewMemoria(nuevoMemoria);
                }
                //verificar lista
                for (i = 0; i < dataMemoria->listaMemorias->elements_count; ++i) {
                    auxMemoria = list_get(dataMemoria->listaMemorias, i);
                    if (configMemoria->NRO_MEMORIA != auxMemoria->numero && !existeMemoria(auxMemoria->numero)) {
                        addNewMemoria(auxMemoria);
                    } else {
                        destroyMemoria(auxMemoria);
                    }
                }
                list_destroy(dataMemoria->listaMemorias);
                free(dataMemoria);
                free(buffer);
            }
        }

    }
}

void cleanMemoria() {
    int i = 0;
    pthread_mutex_lock(&mutex);
    st_memoria *memoriaAux = list_remove(listaTablas, i);
    while (memoriaAux) {
        destroyMemoria(memoriaAux);
        ++i;
        memoriaAux = list_remove(listaTablas, i);
    }
    list_clean(listaTablas);
    pthread_mutex_unlock(&mutex);
}

void removeMemoriaFallida(st_memoria * memoria){
    int i;
    st_memoria * auxMemoria;
    pthread_mutex_lock(&mutex);
    for (i = 0; i < listaTablas->elements_count ; ++i) {
        auxMemoria = list_get(listaTablas,i);
        if(strcmp(memoria->ip,auxMemoria->ip) == 0 && strcmp(memoria->puerto,auxMemoria->puerto) == 0){
            list_remove(listaTablas,i);
            destroyMemoria(auxMemoria);
        }
    }
    pthread_mutex_unlock(&mutex);
}

void *devolverListaMemoria(size_t *size_paquetes) {
    st_data_memoria *memoria = malloc(sizeof(st_data_memoria));
    memoria->numero = configMemoria->NRO_MEMORIA;
    memoria->listaMemorias = listaTablas;
    pthread_mutex_lock(&mutex);
    void * buffer = serealizarMemoria(memoria, size_paquetes);
    pthread_mutex_unlock(&mutex);
    free(memoria);
    return buffer;
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
        cleanMemoria();
        cleanSeed();
        showMiguel(listaTablas);
        log_info(file_log, "buscando data de otras memorias");
        for (i = 0; i < configMemoria->IP_SEEDS->elements_count; ++i) {
            consultarEstadoMemoria(list_get(configMemoria->IP_SEEDS, i), list_get(configMemoria->PUERTO_SEEDS, i));
        }
        //verificar seeed ingresado
        pthread_mutex_lock(&mutexSeeds);
        for (i = 0; i < seedFallidas->elements_count ; ++i) {
            memoria = list_remove(seedFallidas,i);
            removeMemoriaFallida(memoria);
        }
        pthread_mutex_unlock(&mutexSeeds);
        sleep(configMemoria->TIEMPO_GOSSIPING/1000);
    }
}
