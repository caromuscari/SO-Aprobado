/*
 * hiloInotify.c
 *
 *  Created on: 3 ago. 2019
 *      Author: utnso
 */


#include "hiloInotify.h"
#include <signal.h>
#include "configuracionMemoria.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )
#define BUF_LEN     ( 1024 * EVENT_SIZE )
extern t_log* file_log;
extern t_configuracionMemoria *configMemoria;
extern pthread_mutex_t mutexConfig;
extern int loop;
int file_descriptor;
int watch_descriptor;

void hiloinotify(char* rutaConfig){
    log_info(file_log, "Inicializa hilo inotify");

    while(loop){
        char buffer[BUF_LEN];

        file_descriptor = inotify_init();
        if (file_descriptor < 0) {
            perror("inotify_init");
            log_error(file_log, "Error en lectura de config (Inotify)");
        }

        watch_descriptor = inotify_add_watch(file_descriptor, rutaConfig, IN_MODIFY);

        int length = read(file_descriptor, buffer, BUF_LEN);
        if (length < 0) {
            perror("read");
            log_error(file_log, "Error en lectura de config (Inotify)");
        }

        int offset = 0;

        while (offset < length) {
            struct inotify_event *event = (struct inotify_event *) &buffer[offset];
            if (event->mask & IN_MODIFY) {
                cambioConfiguracion(rutaConfig);
            }

            offset += sizeof (struct inotify_event) + event->len;
        }
        // Cierro el Inotify
        inotify_rm_watch(file_descriptor, watch_descriptor);
        close(file_descriptor);
    }
}

void cambioConfiguracion(char* rutaConfig){
	 t_config *bufferConfig = config_create(rutaConfig);
    pthread_mutex_lock(&mutexConfig);
    if(configMemoria->RETARDO_FS != config_get_int_value(bufferConfig, "RETARDO_FS")){
    	configMemoria->RETARDO_FS = config_get_int_value(bufferConfig, "RETARDO_FS");
        log_info(file_log, "[CONFIG] Cambió el valor de RETARDO FILESYSTEM");
    }

    if(configMemoria->RETARDO_MEM != config_get_int_value(bufferConfig, "RETARDO_MEM")){
    	configMemoria->RETARDO_MEM = config_get_int_value(bufferConfig, "RETARDO_MEM");
        log_info(file_log, "[CONFIG] Cambió el valor de RETARDO de MEMORIA");
    }
    pthread_mutex_unlock(&mutexConfig);

    config_destroy(bufferConfig);
}

int getRetardoMem(){
	int retardoMem;
	pthread_mutex_lock(&mutexConfig);
	retardoMem = configMemoria->RETARDO_MEM;
	pthread_mutex_unlock(&mutexConfig);
	return retardoMem;
}

int getRetardoFile(){
	int retardoFile;
	pthread_mutex_lock(&mutexConfig);
	retardoFile = configMemoria->RETARDO_FS;
	pthread_mutex_unlock(&mutexConfig);
	return retardoFile;

}
