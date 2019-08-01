#include "configuracionMemoria.h"

t_configuracionMemoria * leerConfiguracion(char* path){
    t_config *bufferConfig = config_create(path);
    if (!bufferConfig) {
    	printf("No se pudo abrir el archivo de configuracion \n ");
        return NULL;
    }
    t_configuracionMemoria *configuracion = malloc(sizeof(t_configuracionMemoria));
    configuracion->PUERTO = config_get_string_value(bufferConfig, "PUERTO");
    configuracion->TIEMPO_GOSSIPING = config_get_int_value(bufferConfig, "TIEMPO_GOSSIPING");
    //Hay una funcion para ints
    configuracion->NRO_MEMORIA = config_get_int_value(bufferConfig, "NRO_MEMORIA");
    configuracion->IP_MEMORIA = config_get_string_value(bufferConfig, "IP_MEMORIA");
    configuracion->IP_FS = config_get_string_value(bufferConfig, "IP_FS");
    configuracion->PUERTO_FS = config_get_string_value(bufferConfig, "PUERTO_FS");
    configuracion->RETARDO_FS = config_get_int_value(bufferConfig, "RETARDO_FS");
    configuracion->RETARDO_MEM = config_get_int_value(bufferConfig, "RETARDO_MEM");
    configuracion->TIEMPO_JOURNAL = config_get_int_value(bufferConfig, "TIEMPO_JOURNAL");
    configuracion->SLEEP = config_get_int_value(bufferConfig, "SLEEP");
    configuracion->TAM_MEM = config_get_int_value(bufferConfig, "TAM_MEM");

    char **split = config_get_array_value(bufferConfig, "IP_SEEDS");
    int i = 0;
    configuracion->IP_SEEDS = list_create();
    while (split[i]) {
        list_add(configuracion->IP_SEEDS, strdup(split[i]));
        i++;
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
