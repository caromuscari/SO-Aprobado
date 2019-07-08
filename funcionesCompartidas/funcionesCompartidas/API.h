//
// Created by miguelchauca on 21/04/19.
//

#ifndef KERNEL_API_H
#define KERNEL_API_H

#include <commons/string.h>
#include <stdlib.h>
#include <string.h>
#include "time.h"
#include <stdint.h>

enum OPERACION {
    INSERT = 1,
    SELECT = 2,
    CREATE = 3,
    DROP = 4,
    DESCRIBE = 5,
	DESCRIBEGLOBAL = 6,
	EXIT = 7,
	JOURNAL = 8
};

typedef struct {
    enum OPERACION operacion;
    void *instruccion;
} stinstruccion;

typedef struct {
    enum OPERACION operacion;
    char *nameTable;
    char *tipoConsistencia;
    int numeroParticiones;
    int compactionTime;
} st_create;

typedef struct {
    size_t operacion;
    size_t nameTable;
    size_t tipoConsistencia;
    size_t numeroParticiones;
    size_t compactionTime;
} __attribute__((packed)) st_size_create;

typedef struct {
    enum OPERACION operacion;
    char *nameTable;
    uint16_t key;
} st_select;

typedef struct {
    size_t operacion;
    size_t nameTable;
    size_t key;
}__attribute__((packed)) st_size_select;

typedef struct {
    enum OPERACION operacion;
    char *nameTable;
    uint16_t key;
    char *value;
    double timestamp;
} st_insert;

typedef struct {
    size_t size_operacion;
    size_t size_nametable;
    size_t size_key;
    size_t size_value;
    size_t size_timestamp;
}__attribute__((packed)) st_size_insert;

typedef struct {
    enum OPERACION operacion;
    char *nameTable;
} st_describe;

typedef struct {
    size_t operacion;
    size_t nameTable;
} __attribute__((packed)) st_size_describe;

typedef struct {
    enum OPERACION operacion;
    char *nameTable;
} st_drop;

typedef struct {
    size_t operacion;
    size_t nameTable;
}__attribute__((packed)) st_size_drop;

int getEnumFromString(char *string);

void destroySplit(char **split);

//select
st_select *cargarSelect(char *comando);

void destoySelect(st_select *_select);

void *serealizarSelect(st_select *_select, size_t *size_buffer);

st_select *deserealizarSelect(void *buffer);

//create
st_create *cargarCreate(char *comando);

void destroyCreate(st_create *create);

void *serealizarCreate(st_create *create, size_t *size_buffer);

st_create *deserealizarCreate(void *buffer);

//insert
st_insert *cargarInsert(char *comando);

void *serealizarInsert(st_insert *insert, size_t *length);

st_insert *desserealizarInsert(void *buffer);

void destroyInsert(st_insert *insert);

//describe
st_describe *cargarDescribe(char *comando);

void destroyDescribe(st_describe *describe);

void *serealizarDescribe(st_describe *describe, size_t *size_buffer);

st_describe *deserealizarDescribe(void *buffer);

//drop
st_drop *cargarDrop(char *comando);

void destroyDrop(st_drop *drop);

void *serealizarDrop(st_drop *drop, size_t *size_buffer);

st_drop *deserealizarDrop(void *buffer);

#endif //KERNEL_API_H
