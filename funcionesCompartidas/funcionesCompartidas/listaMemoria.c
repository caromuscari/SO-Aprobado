//
// Created by miguelchauca on 05/05/19.
//

#include "listaMemoria.h"

void *serealizarDataMemoria(st_data_memoria *memoria, size_t *size_buffer)
{
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

st_data_memoria *deserealizarDataMemoria(void *buffer, size_t *size_buffer)
{
    st_size_memoria sizeMemoria;
    st_data_memoria *dataMemoria = malloc(sizeof(st_data_memoria));
    int offset = 0;
    memcpy(&sizeMemoria, buffer, sizeof(sizeMemoria));
    offset += sizeof(sizeMemoria);

    dataMemoria->ip = malloc(sizeMemoria.ip);
    memcpy(dataMemoria->ip, (buffer + offset), sizeMemoria.ip);
    offset += sizeMemoria.ip;

    dataMemoria->puerto = malloc(sizeMemoria.puerto);
    memcpy(dataMemoria->puerto, (buffer + offset), sizeMemoria.puerto);
    offset += sizeMemoria.puerto;

    memcpy(&dataMemoria->numero,(buffer + offset), sizeMemoria.numero);
    offset += sizeMemoria.numero;

    if (size_buffer)
    {
        *size_buffer = offset;
    }
    return dataMemoria;
}

void *sereliazarListaDataMemoria(t_list *listaDeMemorias, size_t *size_buffer)
{
    st_data_memoria *memoria;
    int i;
    int offset = 0;
    void *bufferAux = NULL;
    void *buffer = NULL;
    size_t * sizebufferAux;
    size_t sizebuffer = 0;
    t_list *listaBuffes = list_create();
    t_list *listaSizeBuffer = list_create();
    for (i = 0; i < listaDeMemorias->elements_count; i++)
    {
        sizebufferAux = malloc(sizeof(size_t));
        *sizebufferAux = 0;
        memoria = list_get(listaDeMemorias, i);
        bufferAux = serealizarDataMemoria(memoria, sizebufferAux);
        sizebuffer += *sizebufferAux;
        list_add(listaSizeBuffer, sizebufferAux);
        list_add(listaBuffes, bufferAux);
    }
    //juntar todos los pedacitos del buffer
    buffer = malloc(sizebuffer);
    for (i = 0; i < listaBuffes->elements_count; i++)
    {
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

t_list *deserealizarListaDataMemoria(void *buffer, size_t size_buffer)
{
    t_list *listaMemoria = list_create();
    size_t offset = 0;
    size_t sizeBufferAux = 0;
    st_data_memoria *memoria;
    while (offset < size_buffer)
    {
        memoria = deserealizarDataMemoria((buffer + offset), &sizeBufferAux);
        list_add(listaMemoria, memoria);
        offset += sizeBufferAux;
    }
    return listaMemoria;
}

void destroyMemoria(st_data_memoria *memoria)
{
    free(memoria->ip);
    free(memoria->puerto);
    free(memoria);
}

void destroyListaDataMemoria(t_list *listaDeMemorias)
{
    int i;
    st_data_memoria *memoria;
    for (i = 0; i < listaDeMemorias->elements_count; i++)
    {
        memoria = list_get(listaDeMemorias, i);
        destroyMemoria(memoria);
    }
    list_destroy(listaDeMemorias);
}
