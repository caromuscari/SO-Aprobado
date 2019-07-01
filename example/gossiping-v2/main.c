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

typedef struct {
    char *PUERTO;
    t_list *IP_SEEDS;
    t_list *PUERTO_SEEDS;
    int RETARDO_GOSSIPING;
    int MEMORY_NUMBER;
} config;
typedef struct {
    st_data_memoria *memoria;
    bool activo;
} st_tabla_gossip;

config *file_config;
t_list *listaTablas;
t_log *file_log;

config *load_config(char *path) {
    t_config *bufferConfig = config_create(path);
    if (!bufferConfig) {
        return NULL;
    }
    config *configuracion = malloc(sizeof(config));
    configuracion->PUERTO = strdup(config_get_string_value(bufferConfig, "PUERTO"));
    configuracion->RETARDO_GOSSIPING = atoi(config_get_string_value(bufferConfig, "RETARDO_GOSSIPING"));
    configuracion->MEMORY_NUMBER = atoi(config_get_string_value(bufferConfig, "MEMORY_NUMBER"));

    char **split = string_split(config_get_string_value(bufferConfig, "IP_SEEDS"), "\"");
    configuracion->IP_SEEDS = list_create();
    list_add(configuracion->IP_SEEDS, strdup(split[1]));
    list_add(configuracion->IP_SEEDS, strdup(split[3]));
    string_iterate_lines(split, (void *) free);
    free(split);

    split = string_split(config_get_string_value(bufferConfig, "PUERTO_SEEDS"), ",");
    char *subString;
    configuracion->PUERTO_SEEDS = list_create();

    subString = string_substring(split[0], 1, strlen(split[0]) - 1);
    list_add(configuracion->PUERTO_SEEDS, strdup(subString));
    free(subString);

    subString = string_substring(split[1], 0, strlen(split[0]) - 1);
    list_add(configuracion->PUERTO_SEEDS, strdup(subString));
    free(subString);

    string_iterate_lines(split, (void *) free);
    free(split);


    config_destroy(bufferConfig);
    return configuracion;
}

void loadSeed() {
    int i;
    char *dataText;
    for (i = 0; i < file_config->PUERTO_SEEDS->elements_count; ++i) {
        st_tabla_gossip *tabla = malloc(sizeof(st_tabla_gossip));
        tabla->memoria = malloc(sizeof(st_data_memoria));
        tabla->activo = false;

        dataText = list_get(file_config->PUERTO_SEEDS, i);
        tabla->memoria->puerto = strdup(dataText);

        dataText = list_get(file_config->IP_SEEDS, i);
        tabla->memoria->ip = strdup(dataText);
        if (i == 0) {
            tabla->memoria->numero = file_config->MEMORY_NUMBER;
            tabla->activo = true;
        } else {
            tabla->memoria->numero = 0;
        }
        list_add(listaTablas, tabla);
    }
}

void detroyTablaSeed() {
    st_tabla_gossip *tabla;
    for (int i = 0; i < listaTablas->elements_count; ++i) {
        tabla = list_get(listaTablas, i);
        free(tabla->memoria->puerto);
        free(tabla->memoria->ip);
        free(tabla->memoria);
        free(tabla);
    }
    list_destroy(listaTablas);
}


void consultarEstadoTabla(st_tabla_gossip *tabla) {
    int control = 0;
    int i;
    st_data_memoria *dataMemoria;
    int fdClient = establecerConexion(tabla->memoria->ip, tabla->memoria->puerto, file_log, &control);
    if (control != 0) {
        //fallo conexcion
        tabla->activo = false;
    } else {
        //enviar mensaje
        header request;
        request.letra = 'M';
        request.codigo = 1;
        request.sizeData = 1;
        void *paquete = createMessage(&request, " ");
        if (enviar_message(fdClient, paquete, file_log, &control) < 0) {
            tabla->activo = false;
        }
        free(paquete);
        //recibir  respuesta
        control = 0;
        header response;
        paquete = getMessage(fdClient, &response, &control);
        if (paquete == NULL) {
            tabla->activo = false;
        } else {
            t_list *listaTablasConocidas = deserealizarListaDataMemoria(paquete, response.sizeData);
            for (i = 0; i < listaTablasConocidas->elements_count; ++i) {
                dataMemoria = list_get(listaTablasConocidas, i);

            }
        }
    }

}

void *pthreadGossping() {
    listaTablas = list_create();
    loadSeed();
    int i;
    while (true) {
        for (i = 1; i < listaTablas->elements_count; ++i) {
            consultarEstadoTabla(list_get(listaTablas, i));
        }
    }
}

int main(int argc, char **argv) {
    file_log = crear_archivo_log("Kernel", true, "./kernelLog");
    char *pathConfig = argv[1];
    file_config = load_config(pathConfig);
    pthreadGossping();
    return 0;
}