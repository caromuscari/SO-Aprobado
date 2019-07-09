//
// Created by miguelchauca on 05/05/19.
//

#ifndef KERNEL_LISTAMEMORIA_H
#define KERNEL_LISTAMEMORIA_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>

typedef struct {
    char *ip;
    char *puerto;
    int numero;
} st_memoria;

typedef struct {
    int numero;
    t_list *listaMemorias;
} st_data_memoria;

typedef struct {
    size_t ip;
    size_t puerto;
    size_t numero;
} __attribute__((packed)) st_size_memoria;
void *serealizarDataMemoria(st_memoria *memoria, size_t *size_buffer);
st_memoria *deserealizarDataMemoria(void *buffer, size_t *size_buffer);
st_data_memoria *deserealizarMemoria(void *buffer, size_t size_buffer);
void destroyMemoria(st_memoria *memoria);
void destroyListaDataMemoria(st_data_memoria * memoria);
void * serealizarMemoria(st_data_memoria * memoria, size_t * size_buffer);

#endif //KERNEL_LISTAMEMORIA_H
