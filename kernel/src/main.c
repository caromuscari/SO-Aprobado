#include <stdio.h>
#include <funcionesCompartidas/log.h>
#include "load_config.h"
#include "consola.h"
#include "threadMetadata.h"
#include "threadPoolMemoria.h"
#include "threadPlanificador.h"
#include <commons/collections/list.h>
#include <pthread.h>


t_log *file_log;
config * configuracion;
pthread_t plafinifcador;
pthread_t pthreadMemoria;
pthread_t pthreadMetadata;


int inicializar(char * pathConfig){
    file_log = crear_archivo_log("Kernel", true,"./kernelLog");
    log_info(file_log, "cargando el archivo de configuracion");
    configuracion = load_config(pathConfig);
    if(!configuracion) {
        log_error(file_log, "no se pudo cargar el archivo de configuracion");
        log_destroy(file_log);
        return -1;
    }
    return 0;
}
t_list *cargarListMemoria()
{
    t_list *listaMemoria = list_create();
    st_data_memoria *memoria = malloc(sizeof(st_data_memoria));
    memoria->ip = strdup("127.0.0.1");
    memoria->puerto = strdup("3001");
    memoria->numero = 1;
    list_add(listaMemoria, memoria);
    st_data_memoria *memoria2 = malloc(sizeof(st_data_memoria));
    memoria2->ip = strdup("127.0.0.1.5.6");
    memoria2->puerto = strdup("3002");
    memoria2->numero = 2;
    list_add(listaMemoria, memoria2);
    return listaMemoria;
}

t_list *cargarListMemoria2()
{
    t_list *listaMemoria = list_create();
    st_data_memoria *memoria = malloc(sizeof(st_data_memoria));
    memoria->ip = strdup("127.0.0.1");
    memoria->puerto = strdup("3001");
    memoria->numero = 1;
    list_add(listaMemoria, memoria);
    return listaMemoria;
}

int main(int argc, char **argv){
    if(inicializar(argv[1]) < 0){
        return -1;
    }
//    t_list * lista = cargarListMemoria();
//    updateListaMemorias(lista);
//    lista = cargarListMemoria2();
//    updateListaMemorias(lista);
    pthread_create(&plafinifcador,NULL, inicialPlanificador, NULL);
    pthread_create(&pthreadMemoria,NULL,loadPoolMemori,NULL);
    pthread_create(&pthreadMetadata,NULL,schedulerMetadata, NULL);
    consola();
    pthread_cancel(plafinifcador);
    pthread_cancel(pthreadMemoria);
    pthread_cancel(pthreadMetadata);
    log_destroy(file_log);
    return 0;
}