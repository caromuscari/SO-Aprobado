//
// Created by miguelchauca on 30/06/19.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <funcionesCompartidas/listaMemoria.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <pthread.h>

typedef struct {
    char *PUERTO;
    t_list *IP_SEEDS;
    t_list *PUERTO_SEEDS;
    int RETARDO_GOSSIPING;
    int MEMORY_NUMBER;
} config;

config *file_config;
t_list *listaTablas;
t_log *file_log;
t_list * seedFallidas;
pthread_mutex_t mutex;

typedef enum {
    DEVOLVERLISTADEMEMORIA = 7,
    BUSCARLISTADEMEMORIA = 8
} enum_mesanje_memoria;

config *load_config(char *path) {
    t_config *bufferConfig = config_create(path);
    if (!bufferConfig) {
        return NULL;
    }
    config *configuracion = malloc(sizeof(config));
    configuracion->PUERTO = strdup(config_get_string_value(bufferConfig, "PUERTO"));
    configuracion->RETARDO_GOSSIPING = config_get_int_value(bufferConfig, "RETARDO_GOSSIPING");
    configuracion->MEMORY_NUMBER = config_get_int_value(bufferConfig, "MEMORY_NUMBER");

    char **split = config_get_array_value(bufferConfig, "IP_SEEDS");
    int i = 0;
    configuracion->IP_SEEDS = list_create();
    while (split[i]) {
        list_add(configuracion->IP_SEEDS, strdup(split[i]));
        ++i;
    }
    string_iterate_lines(split, (void *) free);
    free(split);

    split = config_get_array_value(bufferConfig, "PUERTO_SEEDS");
    i = 0;
    configuracion->PUERTO_SEEDS = list_create();
    while (split[i]) {
        list_add(configuracion->PUERTO_SEEDS, strdup(split[i]));
        ++i;
    }
    string_iterate_lines(split, (void *) free);
    free(split);


    config_destroy(bufferConfig);
    return configuracion;
}

bool existeMemoria(int numeroMemoria) {
    int i;
    st_memoria *memoria;
    for (i = 0; i < listaTablas->elements_count; ++i) {
        memoria = list_get(listaTablas, i);
        if (memoria->numero == numeroMemoria) {
            return true;
        }
    }
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
    list_add(seedFallidas,memoria);
}

void cleanSeed(){
    int i;
    st_memoria * memoria;
    for (i = 0; i < seedFallidas->elements_count ; ++i) {
        memoria = list_get(seedFallidas,i);
        destroyMemoria(memoria);
    }
    list_clean(seedFallidas);
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
        log_error(file_log, "no se pudo extableser conexion");
        addSeedFallidas(ip,puerto);
        return;
    } else {
        header request;
        request.letra = 'M';
        request.codigo = BUSCARLISTADEMEMORIA;
        request.sizeData = 1;
        void *paquete = createMessage(&request, " ");
        if (enviar_message(fdClient, paquete, file_log, &control) < 0) {
            return;
        } else {
            control = 0;
            header response;
            paquete = getMessage(fdClient, &response, &control);
            if (paquete == NULL) {
                return;
            } else {
                dataMemoria = deserealizarMemoria(paquete, response.sizeData);
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
                    if (file_config->MEMORY_NUMBER != auxMemoria->numero && !existeMemoria(auxMemoria->numero)) {
                        addNewMemoria(auxMemoria);
                    } else {
                        destroyMemoria(auxMemoria);
                    }
                }
                list_destroy(dataMemoria->listaMemorias);
                free(dataMemoria);
            }
        }

    }
}

void cleanMemoria() {
    int i = 0;
    pthread_mutex_lock(&mutex);
    st_memoria *memoriaAux = list_get(listaTablas, i);
    while (memoriaAux) {
        destroyMemoria(memoriaAux);
        ++i;
        memoriaAux = list_get(listaTablas, i);
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

void *pthreadGossping() {
    listaTablas = list_create();
    seedFallidas = list_create();
    st_memoria * memoria;
    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return NULL;
    }
    int i;
    while (true) {
        cleanMemoria();
        cleanSeed();
        showMiguel(listaTablas);
        log_info(file_log, "buscando data de otras memorias");
        for (i = 0; i < file_config->IP_SEEDS->elements_count; ++i) {
            consultarEstadoMemoria(list_get(file_config->IP_SEEDS, i), list_get(file_config->PUERTO_SEEDS, i));
        }
        //verificar seeed ingresado
        for (i = 0; i < seedFallidas->elements_count ; ++i) {
            memoria = list_get(seedFallidas,i);
            removeMemoriaFallida(memoria);
        }
        printf("------Memoria [%d]-------\n", file_config->MEMORY_NUMBER);
        showMiguel(listaTablas);
        sleep(8);
    }
}

void *devolverListaMemoria(size_t *size_paquetes) {
    st_data_memoria *memoria = malloc(sizeof(st_data_memoria));
    memoria->numero = file_config->MEMORY_NUMBER;
    memoria->listaMemorias = listaTablas;
    pthread_mutex_lock(&mutex);
    void * buffer = serealizarMemoria(memoria, size_paquetes);
    pthread_mutex_unlock(&mutex);
    return buffer;
}

void *pthreadServer() {
    int control = 0;
    int server = makeListenSock(file_config->PUERTO, file_log, &control);
    log_info(file_log, "Inicializando server");
    if (server == -1) {
        log_error(file_log, "Error al crear el server");
    }
    while (true) {
        control = 0;
        log_info(file_log, "esperando entrada nueva");
        int fdClient = aceptar_conexion(server, file_log, &control);
        if (control != 0) {
            log_error(file_log, "no se puede aceptar la conexion");
            continue;
        }
        // leer mensaje
        header request;
        log_info(file_log, "esperando mensaje");
        void *paquetes = getMessage(fdClient, &request, &control);
        if (paquetes == NULL) {
            log_error(file_log, "error al obtener el mensaje");
        }
        if (request.letra == 'M') {
            switch (request.codigo) {
                case BUSCARLISTADEMEMORIA: {
                    log_info(file_log, "enviando respuesta");
                    header response;
                    size_t sizePaqueteRes = 0;
                    void *paqueteLista = devolverListaMemoria(&sizePaqueteRes);
                    response.codigo = DEVOLVERLISTADEMEMORIA;
                    response.letra = 'M';
                    response.sizeData = sizePaqueteRes;
                    void *paqueteRespuesta = createMessage(&response, paqueteLista);
                    if (enviar_message(fdClient, paqueteRespuesta, file_log, &control) < 0) {
                        log_error(file_log, "no se pudo enviar la respuesta solicitada");
                        continue;
                    }
                }
            }
        }
        if (request.letra == 'K') {
            switch (request.codigo) {
                case BUSCARLISTADEMEMORIA: {
                    log_info(file_log, "enviando respuesta");
                    header response;
                    size_t sizePaqueteRes = 0;
                    void *paqueteLista = devolverListaMemoria(&sizePaqueteRes);
                    response.codigo = DEVOLVERLISTADEMEMORIA;
                    response.letra = 'M';
                    response.sizeData = sizePaqueteRes;
                    void *paqueteRespuesta = createMessage(&response, paqueteLista);
                    if (enviar_message(fdClient, paqueteRespuesta, file_log, &control) < 0) {
                        log_error(file_log, "no se pudo enviar la respuesta solicitada");
                        continue;
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    file_log = crear_archivo_log("Kernel", true, "./kernelLog");
    char *pathConfig = argv[1];
    file_config = load_config(pathConfig);
    pthread_t gossping;
    pthread_t server;
    pthread_create(&gossping, NULL, pthreadGossping, NULL);
    pthread_create(&server, NULL, pthreadServer, NULL);
    pthread_join(gossping, NULL);
    pthread_join(server, NULL);
    return 0;
}