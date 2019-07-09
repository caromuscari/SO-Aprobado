//
// Created by miguelchauca on 05/05/19.
//

#include "listaMemoria.h"

void *serealizarDataMemoria(st_memoria *memoria, size_t *size_buffer) {
    st_size_memoria sizeMemoria;
    sizeMemoria.ip = strlen(memoria->ip) + 1;
    sizeMemoria.puerto = strlen(memoria->puerto) + 1;
    sizeMemoria.numero = sizeof(memoria->numero);

    size_t sizeBuffer = sizeMemoria.ip + sizeMemoria.puerto + sizeMemoria.numero + sizeof(sizeMemoria);
    *size_buffer = sizeBuffer;
    void *buffer = malloc(sizeBuffer);
    int offset = 0;

    memcpy(buffer, &sizeMemoria, sizeof(sizeMemoria));
    offset += sizeof(sizeMemoria);

    memcpy((buffer + offset), memoria->ip, sizeMemoria.ip);
    offset += sizeMemoria.ip;

    memcpy((buffer + offset), memoria->puerto, sizeMemoria.puerto);
    offset += sizeMemoria.puerto;

    memcpy((buffer + offset), &memoria->numero, sizeMemoria.numero);

    return buffer;
}

st_memoria *deserealizarDataMemoria(void *buffer, size_t *size_buffer) {
    st_size_memoria sizeMemoria;
    st_memoria *dataMemoria = malloc(sizeof(st_memoria));
    int offset = 0;
    memcpy(&sizeMemoria, buffer, sizeof(sizeMemoria));
    offset += sizeof(sizeMemoria);

    dataMemoria->ip = malloc(sizeMemoria.ip);
    memcpy(dataMemoria->ip, (buffer + offset), sizeMemoria.ip);
    offset += sizeMemoria.ip;

    dataMemoria->puerto = malloc(sizeMemoria.puerto);
    memcpy(dataMemoria->puerto, (buffer + offset), sizeMemoria.puerto);
    offset += sizeMemoria.puerto;

    memcpy(&dataMemoria->numero, (buffer + offset), sizeMemoria.numero);
    offset += sizeMemoria.numero;

    if (size_buffer) {
        *size_buffer = offset;
    }
    return dataMemoria;
}

st_data_memoria *deserealizarMemoria(void *buffer, size_t size_buffer) {
    st_data_memoria * dataMemoria = malloc(sizeof(st_memoria));
    dataMemoria->listaMemorias = list_create();
    size_t offset = sizeof(int);
    memcpy(&dataMemoria->numero,buffer, sizeof(int));
    size_t sizeBufferAux = 0;
    st_memoria *memoria;
    while (offset < size_buffer) {
        memoria = deserealizarDataMemoria((buffer + offset), &sizeBufferAux);
        list_add(dataMemoria->listaMemorias, memoria);
        offset += sizeBufferAux;
    }
    return dataMemoria;
}

void destroyMemoria(st_memoria *memoria) {
    free(memoria->ip);
    free(memoria->puerto);
    free(memoria);
}

void destroyListaDataMemoria(st_data_memoria * memoria) {
    int i;
    st_memoria *memoriaAux;
    for (i = 0; i < memoria->listaMemorias->elements_count; i++) {
        memoriaAux = list_get(memoria->listaMemorias, i);
        destroyMemoria(memoriaAux);
    }
    list_destroy(memoria->listaMemorias);
    free(memoria);
}

void * serealizarMemoria(st_data_memoria * memoria, size_t * size_buffer){
    st_memoria *memoriaAux;
    int i;
    int offset = sizeof(int);
    void *bufferAux = NULL;
    void *buffer = NULL;
    size_t *sizebufferAux;
    size_t sizebuffer = sizeof(int);
    t_list *listaBuffes = list_create();
    t_list *listaSizeBuffer = list_create();
    for (i = 0; i < memoria->listaMemorias->elements_count; i++) {
        sizebufferAux = malloc(sizeof(size_t));
        *sizebufferAux = 0;
        memoriaAux = list_get(memoria->listaMemorias, i);
        bufferAux = serealizarDataMemoria(memoriaAux, sizebufferAux);
        sizebuffer += *sizebufferAux;
        list_add(listaSizeBuffer, sizebufferAux);
        list_add(listaBuffes, bufferAux);
    }
    //juntar todos los pedacitos del buffer
    buffer = malloc(sizebuffer);
    memcpy(buffer,&memoria->numero, sizeof(int));
    for (i = 0; i < listaBuffes->elements_count; i++) {
        sizebufferAux = list_get(listaSizeBuffer, i);
        bufferAux = list_get(listaBuffes, i);
        memcpy((buffer + offset), bufferAux, *sizebufferAux);
        offset += *sizebufferAux;
        free(bufferAux);
        free(sizebufferAux);
    }
    *size_buffer = sizebuffer;
    list_destroy(listaBuffes);
    list_destroy(listaSizeBuffer);
    return buffer;


}
