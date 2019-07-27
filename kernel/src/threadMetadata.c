//
// Created by miguelchauca on 28/04/19.
//

#include "threadMetadata.h"
#include <pthread.h>
#include <commons/string.h>

t_list *listMetadata = NULL;
extern config *configuracion;
t_log *file_log_metada;
pthread_mutex_t mutex;

void logStatusListaMetadata() {
    st_metadata *metadata;
    log_info(file_log_metada, "----Estado de MetaData---");
    for (int i = 0; i < listMetadata->elements_count; ++i) {
        metadata = list_get(listMetadata, i);
        log_info(file_log_metada, "Nametable = [%s]", metadata->nameTable);
        log_info(file_log_metada, "Consistencia [%s]", metadata->consistency);
    }
}

int getCriterioByNameTabla(char *nameTable) {
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
        return NO_SE_ENCONTRO_TABLA;
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

void removeTablaByName(char *nameTable) {
    log_info(file_log_metada, "[MetaData] Eliminando metadata [%s]",nameTable);
    st_metadata *metadata;
    pthread_mutex_lock(&mutex);
    int pos = buscarNameTable(nameTable);
    metadata = list_remove(listMetadata, pos);
    pthread_mutex_unlock(&mutex);
    destroyMetaData(metadata);
}

void addNuevaTabla(st_metadata *metadata) {
    pthread_mutex_lock(&mutex);
    if (buscarNameTable(metadata->nameTable) == -1) {
        log_info(file_log_metada, "[MetaData] Agregando nueva metadata [%s]",metadata->nameTable);
        list_add(listMetadata, metadata);
    }
    pthread_mutex_unlock(&mutex);
}

void updateListaMetadata(t_list *nuevaLista) {
    log_info(file_log_metada, "[MetaData] Actulizando Lista");
    pthread_mutex_lock(&mutex);
    if (listMetadata != NULL) {
        destroyListaMetaData(listMetadata);
    }
    listMetadata = nuevaLista;
    pthread_mutex_unlock(&mutex);
}

void *schedulerMetadata() {
    listMetadata = list_create();
    file_log_metada = crear_archivo_log("Metadata", false, "./Metadata.log");
    st_messageResponse *respuestaMesanje = NULL;
    void *buffer = NULL;
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("\n mutex init failed\n");
        pthread_exit(NULL);
    }
    while (1) {
        log_info(file_log_metada, "[MetaData] buscando metadata");
        buffer = strdup("1");
        respuestaMesanje = consultarAMemoria(configuracion->IP_MEMORIA, configuracion->PUERTO_MEMORIA, DESCRIBEGLOBAL,
                                             buffer, 1);
        if (respuestaMesanje) {
            switch (respuestaMesanje->cabezera.codigo) {
                case SUCCESS: {
                	t_list * lista = deserealizarListaMetaData(respuestaMesanje->buffer, respuestaMesanje->cabezera.sizeData);
                    updateListaMetadata(lista);
                    destroyStMessageResponse(respuestaMesanje);
                    break;
                }
                case NOSUCCESS: {
                    log_info(file_log_metada, "[MetaData] no hay resultado");
                    destroyStMessageResponse(respuestaMesanje);
                    break;
                }
                default: {
                    log_info(file_log_metada, "[MetaData] no entiendo el codigo re respuesta");
                    destroyStMessageResponse(respuestaMesanje);
                    break;
                }
            }
        } else {
            log_error(file_log_metada, "[MetaData] No hubo respuesta en el DESCRIBE GLOBAL");
        }
        free(buffer);
        logStatusListaMetadata();
        sleep(configuracion->METADATA_REFRESH / 1000);
    }
}
