//
// Created by miguelchauca on 20/04/19.
//

#ifndef KERNEL_LOAD_CONFIG_H
#define KERNEL_LOAD_CONFIG_H

#include <commons/config.h>
#include <funcionesCompartidas/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

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
void updateConfiguracion(char* rutaConfig);
int getSLEEP_EJECUCION();
int getMETADATA_REFRESH();
int getQUANTUM();
#endif //KERNEL_LOAD_CONFIG_H
