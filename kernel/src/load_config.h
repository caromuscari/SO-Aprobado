//
// Created by miguelchauca on 20/04/19.
//

#ifndef KERNEL_LOAD_CONFIG_H
#define KERNEL_LOAD_CONFIG_H

#include <commons/config.h>
#include <stdlib.h>
#include <string.h>

typedef  struct {
    char * IP_MEMORIA;
    char * PUERTO_MEMORIA;
    int QUANTUM;
    int MULTIPROCESAMIENTO;
    int METADATA_REFRESH;
    int SLEEP_EJECUCION;
    int REFRESH_GOSSIPING;
} config;
config * load_config(char * path);

#endif //KERNEL_LOAD_CONFIG_H
