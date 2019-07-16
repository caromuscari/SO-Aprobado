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
    if (result) {
        return getTipoCriterioByString(result->consistency);
    } else {
        return -1;
    }
}

int buscarNameTable(char *nameTable) {
    int i;
    st_metadata *metadata;
    for (i = 0; i < listMetadata->elements_count; ++i) {
        metadata = list_get(listMetadata, i);
        if (strcmp(metadata->nameTable, nameTable) == 0) {
            return i;
        }
    }
    return -1;
}

void removeTablaByName(char * nameTable){
    st_metadata * metadata;
    pthread_mutex_lock(&mutex);
    int pos = buscarNameTable(nameTable);
    metadata = list_remove(listMetadata,pos);
    pthread_mutex_unlock(&mutex);
    destroyMetaData(metadata);
}

void addNuevaTabla(st_metadata * metadata){
    pthread_mutex_lock(&mutex);
    list_add(listMetadata,metadata);
    pthread_mutex_unlock(&mutex);
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
    listMetadata = list_create();
    st_messageResponse * respuestaMesanje = NULL;
    void *buffer = NULL;
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("\n mutex init failed\n");
        pthread_exit(NULL);
    }
    while (1) {
        log_info(file_log,"[MetaData] buscando metadata\n");
        buffer = strdup("1");
        respuestaMesanje = consultarAMemoria(configuracion->IP_MEMORIA, configuracion->PUERTO_MEMORIA,DESCRIBEGLOBAL,buffer,1);
        if(respuestaMesanje){
            switch (respuestaMesanje->cabezera.codigo){
                case SUCCESS:{
                    updateListaMetadata(deserealizarListaMetaData(respuestaMesanje->buffer, respuestaMesanje->cabezera.sizeData));
                    destroyStMessageResponse(respuestaMesanje);
                    break;
                }
                default:{
                    log_info(file_log,"[MetaData] no entiendo el codigo re respuesta\n");
                    break;
                }
            }
        }else{
            log_error(file_log,"[MetaData] No hubo respuesta en el DESRIBE GLOBAL");
        }
        free(buffer);
        sleep(configuracion->METADATA_REFRESH);
    }
}
