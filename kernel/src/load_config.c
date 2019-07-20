//
// Created by miguelchauca on 20/04/19.
//

#include "load_config.h"

config * load_config(char * path){
    t_config *bufferConfig = config_create(path);
    if(!bufferConfig){
        return NULL;
    }
    config * configuracion = malloc(sizeof(config));

    configuracion->IP_MEMORIA = strdup(config_get_string_value(bufferConfig, "IP_MEMORIA"));
    configuracion->PUERTO_MEMORIA = strdup(config_get_string_value(bufferConfig, "PUERTO_MEMORIA"));
    configuracion->QUANTUM =  atoi(config_get_string_value(bufferConfig, "QUANTUM"));
    configuracion->MULTIPROCESAMIENTO = atoi(config_get_string_value(bufferConfig, "MULTIPROCESAMIENTO"));
    configuracion->METADATA_REFRESH = atoi(config_get_string_value(bufferConfig, "METADATA_REFRESH"));
    configuracion->SLEEP_EJECUCION = atoi(config_get_string_value(bufferConfig, "SLEEP_EJECUCION"))/1000;

    config_destroy(bufferConfig);
    return  configuracion;
}
