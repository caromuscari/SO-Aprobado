//
// Created by miguelchauca on 20/04/19.
//

#include "load_config.h"

pthread_mutex_t mutex;
extern config *configuracion;
extern t_log *file_log;

config * load_config(char * path){
    t_config *bufferConfig = config_create(path);
    if(!bufferConfig){
        return NULL;
    }
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("\n [confguracion] mutex init failed\n");
        return NULL;
    }
    config * auxConfig = malloc(sizeof(config));

    auxConfig->IP_MEMORIA = strdup(config_get_string_value(bufferConfig, "IP_MEMORIA"));
    auxConfig->PUERTO_MEMORIA = strdup(config_get_string_value(bufferConfig, "PUERTO_MEMORIA"));
    auxConfig->QUANTUM =  atoi(config_get_string_value(bufferConfig, "QUANTUM"));
    auxConfig->MULTIPROCESAMIENTO = atoi(config_get_string_value(bufferConfig, "MULTIPROCESAMIENTO"));
    auxConfig->METADATA_REFRESH = atoi(config_get_string_value(bufferConfig, "METADATA_REFRESH"));
    auxConfig->SLEEP_EJECUCION = atoi(config_get_string_value(bufferConfig, "SLEEP_EJECUCION"));
    auxConfig->REFRESH_GOSSIPING = config_get_int_value(bufferConfig,"REFRESH_GOSSIPING");
    config_destroy(bufferConfig);
    return  auxConfig;
}

int getQUANTUM(){
    int quantum;
    pthread_mutex_lock(&mutex);
    quantum = configuracion->QUANTUM;
    pthread_mutex_unlock(&mutex);
    return quantum;
}

int getMETADATA_REFRESH(){
    int metedataRefresh;
    pthread_mutex_lock(&mutex);
    metedataRefresh = configuracion->METADATA_REFRESH;
    pthread_mutex_unlock(&mutex);
    return metedataRefresh;
}

int getSLEEP_EJECUCION(){
    int sleepEjecucion;
    pthread_mutex_lock(&mutex);
    sleepEjecucion = configuracion->SLEEP_EJECUCION;
    pthread_mutex_unlock(&mutex);
    return sleepEjecucion;
}

void updateConfiguracion(char* rutaConfig){
    log_info(file_log, "[notify] Iniciando update");
    t_config *bufferConfig;
    bufferConfig = config_create(rutaConfig);
    pthread_mutex_lock(&mutex);
    if(configuracion->QUANTUM != config_get_int_value(bufferConfig,"QUANTUM")){
        configuracion->QUANTUM = config_get_int_value(bufferConfig,"QUANTUM");
    }
    if(configuracion->METADATA_REFRESH != config_get_int_value(bufferConfig,"METADATA_REFRESH")){
        configuracion->METADATA_REFRESH = config_get_int_value(bufferConfig,"METADATA_REFRESH");
    }
    if(configuracion->SLEEP_EJECUCION != config_get_int_value(bufferConfig,"SLEEP_EJECUCION")){
        configuracion->SLEEP_EJECUCION = config_get_int_value(bufferConfig,"SLEEP_EJECUCION");
    }
    pthread_mutex_unlock(&mutex);
    config_destroy(bufferConfig);
    log_info(file_log, "[notify] Finalizando update");
}


