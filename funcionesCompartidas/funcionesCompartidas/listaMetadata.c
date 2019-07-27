//
// Created by miguelchauca on 18/05/19.
//

#include "listaMetadata.h"

void *serealizarMetaData(st_metadata *metadata, size_t *sizeBuffer) {
    st_size_metadata sizeMetadata;
    sizeMetadata.nameTable = strlen(metadata->nameTable) + 1;
    sizeMetadata.consistency = strlen(metadata->consistency) + 1;
    sizeMetadata.partitions = sizeof(metadata->partitions);
    sizeMetadata.compaction_time = sizeof(metadata->compaction_time);

    *sizeBuffer =
            sizeof(sizeMetadata) + sizeMetadata.nameTable + sizeMetadata.consistency + sizeMetadata.partitions +
            sizeMetadata.compaction_time;
    void *buffer = malloc(*sizeBuffer);
    int offset = 0;

    memcpy(buffer, &sizeMetadata, sizeof(sizeMetadata));
    offset += sizeof(sizeMetadata);

    memcpy((buffer + offset), metadata->nameTable, sizeMetadata.nameTable);
    offset += sizeMetadata.nameTable;

    memcpy((buffer + offset), metadata->consistency, sizeMetadata.consistency);
    offset += sizeMetadata.consistency;

    memcpy((buffer + offset), &metadata->partitions, sizeMetadata.partitions);
    offset += sizeMetadata.partitions;

    memcpy((buffer + offset), &metadata->compaction_time, sizeMetadata.compaction_time);

    return buffer;
}

st_metadata *deserealizarMetaData(void *buffer, size_t *sizeBuffer) {
    st_size_metadata sizeMetadata;
    st_metadata *metadata = malloc(sizeof(st_metadata));
    int offset = 0;

    memcpy(&sizeMetadata, buffer, sizeof(sizeMetadata));
    offset += sizeof(sizeMetadata);

    metadata->nameTable = malloc(sizeMetadata.nameTable);
    memcpy(metadata->nameTable, (buffer + offset), sizeMetadata.nameTable);
    offset += sizeMetadata.nameTable;

    metadata->consistency = malloc(sizeMetadata.consistency);
    memcpy(metadata->consistency, (buffer + offset), sizeMetadata.consistency);
    offset += sizeMetadata.consistency;

    memcpy(&metadata->partitions, (buffer + offset), sizeMetadata.partitions);
    offset += sizeMetadata.partitions;

    memcpy(&metadata->compaction_time, (buffer + offset), sizeMetadata.compaction_time);
    offset += sizeMetadata.compaction_time;

    *sizeBuffer = offset;
    return metadata;
}

void destroyMetaData(st_metadata *metadata) {
    free(metadata->nameTable);
    free(metadata->consistency);
    free(metadata);
}

void *serealizarListaMetaData(t_list *listaMetaData, size_t *sizeBuffer) {
    st_metadata *metadata;
    int i;
    int offset = 0;
    t_list *lista_buffer = list_create();
    t_list *lista_size_buffer = list_create();
    *sizeBuffer = 0;
    size_t *size_buffer_metadata;
    void *buffer;
    void *buffer_metadata;
    for (i = 0; i < listaMetaData->elements_count; ++i) {
        size_buffer_metadata = malloc(sizeof(size_t));
        *size_buffer_metadata = 0;
        metadata = list_get(listaMetaData, i);
        buffer_metadata = serealizarMetaData(metadata, size_buffer_metadata);
        *sizeBuffer += *size_buffer_metadata;
        list_add(lista_buffer, buffer_metadata);
        list_add(lista_size_buffer, size_buffer_metadata);
    }

    buffer = malloc(*sizeBuffer);
    for (i = 0; i < lista_buffer->elements_count; ++i) {
        buffer_metadata = list_get(lista_buffer, i);
        size_buffer_metadata = list_get(lista_size_buffer, i);
        memcpy((buffer + offset), buffer_metadata, *size_buffer_metadata);
        offset += *size_buffer_metadata;
        free(buffer_metadata);
        free(size_buffer_metadata);
    }

    list_destroy(lista_buffer);
    list_destroy(lista_size_buffer);
    return buffer;
}

t_list *deserealizarListaMetaData(void *buffer, size_t size_buffer) {
    t_list *lista_metadata = list_create();
    st_metadata *metadata;
    size_t size_copiado = 0;
    size_t offset = 0;
    while (offset < size_buffer) {
        metadata = deserealizarMetaData((buffer + offset), &size_copiado);
        list_add(lista_metadata, metadata);
        offset += size_copiado;
    }
    return lista_metadata;
}

void destroyListaMetaData(t_list *lista_metadata) {
    st_metadata *metadata;
    int i;
    for (i = 0; i < lista_metadata->elements_count; ++i) {
        metadata = list_remove(lista_metadata, i);
        destroyMetaData(metadata);
    }
    list_destroy(lista_metadata);
}
