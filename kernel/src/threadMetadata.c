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

void showLista() {
    int i = 0;
    st_metadata *metadata;
    for (i = 0; i < listMetadata->elements_count; ++i) {
        metadata = list_get(listMetadata, i);
        printf("%s\n", metadata->nameTable);
        printf("%s\n", metadata->consistency);
        printf("%d\n", metadata->partitions);
        printf("%d\n", metadata->compaction_time);
    }
}

char *getTabla(char *nameTable) {
    st_metadata * result = NULL;
    pthread_mutex_lock(&mutex);
    int search_tabla(st_metadata *p) {
        return string_equals_ignore_case(p->nameTable, nameTable);
    }
    result = list_find(listMetadata, (void *) search_tabla);
    pthread_mutex_unlock(&mutex);
    return strdup(result->consistency);
}

void updateListaMetadata(t_list * nuevaLista){
    pthread_mutex_lock(&mutex);
    if (listMetadata != NULL) {
        destroyListaMetaData(listMetadata);
    }
    listMetadata = nuevaLista;
    pthread_mutex_unlock(&mutex);
}

void cleanList(){
    pthread_mutex_lock(&mutex);
    if(listMetadata != NULL){
        destroyListaMetaData(listMetadata);
    }
    list_create(listMetadata);
    pthread_mutex_unlock(&mutex);
}

void *schedulerMetadata() {
    int socketClient;
    int control = 0;
    message *bufferMensaje;
    void *buffer = NULL;
    header request;
    header response;
    bool error;
    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return NULL;
    }
    while (1) {
        error = true;
        socketClient = establecerConexion(configuracion->IP_MEMORIA, configuracion->PUERTO_MEMORIA, file_log, &control);
        log_info(file_log, "Actulizando metadata");
        if (socketClient != -1) {
            request.letra = 'K';
            request.codigo = 6;
            request.sizeData = 1;
            bufferMensaje = createMessage(&request, " ");
            enviar_message(socketClient, bufferMensaje, file_log, &control);
            free(bufferMensaje);
            if (control == 0) {
                buffer = getMessage(socketClient, &response, &control);
                if (buffer) {
                    error = false;
                    updateListaMetadata(deserealizarListaMetaData(buffer, response.sizeData));
                    close(socketClient);
                    free(buffer);
                }
            }
        }
        if (error) {
            cleanList();
        }
        sleep(configuracion->MULTIPROCESAMIENTO);
    }
}
