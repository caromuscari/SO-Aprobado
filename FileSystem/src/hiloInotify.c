//
// Created by utnso on 17/06/19.
//

#include "hiloInotify.h"
#include <signal.h>
#include "Funciones.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )
#define BUF_LEN     ( 1024 * EVENT_SIZE )
extern t_log* alog;
extern structConfig * config;
extern int loop;
int file_descriptor;
int watch_descriptor;

void hiloinotify(char* rutaConfig){
    log_info(alog, "Inicializa hilo inotify");

    while(loop){
        char buffer[BUF_LEN];

        file_descriptor = inotify_init();
        if (file_descriptor < 0) {
            perror("inotify_init");
            log_error(alog, "Error en lectura de config (Inotify)");
        }

        watch_descriptor = inotify_add_watch(file_descriptor, rutaConfig, IN_MODIFY);

        int length = read(file_descriptor, buffer, BUF_LEN);
        if (length < 0) {
            perror("read");
            log_error(alog, "Error en lectura de config (Inotify)");
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
    t_config *configuracion;
    configuracion = config_create(rutaConfig);
    if(config->retardo != config_get_int_value(configuracion, "RETARDO")){
        config->retardo = config_get_int_value(configuracion, "RETARDO");
        log_info(alog, "[CONFIG] Cambió el valor de RETARDO");
    }

    if(config->tiempo_dump != config_get_int_value(configuracion, "TIEMPO_DUMP")){
        config->tiempo_dump = config_get_int_value(configuracion, "TIEMPO_DUMP");
        log_info(alog, "[CONFIG] Cambió el valor de TIEMPO_DUMP");
    }

    config_destroy(configuracion);
}
