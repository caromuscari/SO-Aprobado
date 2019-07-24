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
config *configuracion;
pthread_t plafinifcador;
pthread_t pthreadMemoria;
pthread_t pthreadMetadata;


int inicializar(char *pathConfig) {
    file_log = crear_archivo_log("Kernel", false, "./kernelLog");
    log_info(file_log, "cargando el archivo de configuracion");
    configuracion = load_config(pathConfig);
    if (!configuracion) {
        log_error(file_log, "no se pudo cargar el archivo de configuracion");
        log_destroy(file_log);
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (inicializar(argv[1]) < 0) {
        return -1;
    }
    pthread_create(&plafinifcador,NULL, (void*)inicialPlanificador, NULL);
    pthread_detach(plafinifcador);
    pthread_create(&pthreadMemoria,NULL,(void*)loadPoolMemori,NULL);
    pthread_detach(pthreadMemoria);
    pthread_create(&pthreadMetadata, NULL, (void*)schedulerMetadata, NULL);
    pthread_detach(pthreadMetadata);
    consola();
    pthread_cancel(plafinifcador);
    pthread_cancel(pthreadMemoria);
    pthread_cancel(pthreadMetadata);
    free(configuracion->IP_MEMORIA);
    free(configuracion->PUERTO_MEMORIA);
    free(configuracion);
    log_destroy(file_log);
    return 0;
}
