//
// Created by miguel on 2/8/19.
//

#include "threadNotify.h"
#include <sys/types.h>
#include <sys/inotify.h>
#include <funcionesCompartidas/log.h>
#include "load_config.h"
#include <stdio.h>
#include <unistd.h>

extern t_log *file_log;
#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )
#define BUF_LEN     ( 1024 * EVENT_SIZE )

void hiloinotify(char* rutaConfig){
    log_info(file_log, "Inicializa hilo inotify");
    int file_descriptor;
    int watch_descriptor;
    while(true){
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
                updateConfiguracion(rutaConfig);
            }

            offset += sizeof(struct inotify_event) + event->len;
        }
        // Cierro el Inotify
        inotify_rm_watch(file_descriptor, watch_descriptor);
        close(file_descriptor);
    }
}