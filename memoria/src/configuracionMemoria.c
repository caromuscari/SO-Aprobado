#include "configuracionMemoria.h"

extern t_log *file_log;

t_configuracionMemoria * leerConfiguracion(char* path){
    t_config *bufferConfig = config_create(path);
    if (!bufferConfig) {
        log_error(file_log," ");
        return NULL;
    }
    t_configuracionMemoria *configuracion = malloc(sizeof(t_configuracionMemoria));
    configuracion->PUERTO = strdup(config_get_string_value(bufferConfig, "PUERTO"));
    configuracion->TIEMPO_GOSSIPING = atoi(config_get_string_value(bufferConfig, "TIEMPO_GOSSIPING"));
    configuracion->NRO_MEMORIA = atoi(config_get_string_value(bufferConfig, "NRO_MEMORIA"));
    configuracion->IP_FS = strdup(config_get_string_value(bufferConfig, "IP_FS"));
    configuracion->PUERTO_FS = strdup(config_get_string_value(bufferConfig, "PUERTO_FS"));
    configuracion->RETARDO_FS = atoi(config_get_string_value(bufferConfig, "RETARDO_FS"));
    configuracion->RETARDO_MEM = atoi(config_get_string_value(bufferConfig, "RETARDO_MEM"));
    configuracion->TIEMPO_JOURNAL = atoi(config_get_string_value(bufferConfig, "TIEMPO_JOURNAL"));
    configuracion->SLEEP = atoi(config_get_string_value(bufferConfig, "SLEEP"));
    configuracion->TAM_MEM = atoi(config_get_string_value(bufferConfig, "TAM_MEM"));

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