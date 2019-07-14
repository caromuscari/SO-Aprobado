//
// Created by miguelchauca on 28/04/19.
//

#include "threadMetadata.h"
#include <pthread.h>
#include <commons/string.h>

t_list *listMetadata = NULL;
extern config *configuracion;
extern t_log *file_log;
pthread_mutex_t mutex;

TypeCriterio getCriterioByNameTabla(char *nameTable) {
    st_metadata *result = NULL;
    pthread_mutex_lock(&mutex);
    int search_tabla(st_metadata *p) {
        return string_equals_ignore_case(p->nameTable, nameTable);
    }
    result = list_find(listMetadata, (void *) search_tabla);
    pthread_mutex_unlock(&mutex);
    if(result){
        return getTipoCriterioByString(result->consistency);
    }else{
        return -1;
    }
}

void updateListaMetadata(t_list *nuevaLista) {
    pthread_mutex_lock(&mutex);
    if (listMetadata != NULL) {
        destroyListaMetaData(listMetadata);
    }
    listMetadata = nuevaLista;
    pthread_mutex_unlock(&mutex);
}

void *schedulerMetadata() {
    int socketClient;
    listMetadata = list_create();
    int control = 0;
    message *bufferMensaje = NULL;
    void *buffer = NULL;
    header request;
    header response;
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("\n mutex init failed\n");
        pthread_exit(NULL);
    }
    while (1) {
        socketClient = establecerConexion(configuracion->IP_MEMORIA, configuracion->PUERTO_MEMORIA, file_log, &control);
        if (control != 0) {
            sleep(configuracion->METADATA_REFRESH);
            continue;
        }
        log_info(file_log, "Actulizando metadata");
        request.letra = 'K';
        request.codigo = 6;
        request.sizeData = 0;
        bufferMensaje = createMessage(&request, NULL);
        enviar_message(socketClient, bufferMensaje, file_log, &control);
        if (bufferMensaje->buffer) free(bufferMensaje->buffer);
        free(bufferMensaje);
        if (control == 0) {
            buffer = getMessage(socketClient, &response, &control);
            if (control >= 0) {
                if(response.codigo == 13){
                    updateListaMetadata(deserealizarListaMetaData(buffer, response.sizeData));
                }
                free(buffer);
            }
        }
        close(socketClient);
        sleep(configuracion->METADATA_REFRESH);
    }
}
