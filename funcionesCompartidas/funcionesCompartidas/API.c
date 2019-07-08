//
// Created by miguelchauca on 21/04/19.
//

#include "API.h"

st_insert *cargarInsert(char *comando) {
    char *hayString;
    st_insert *insert = malloc(sizeof(st_insert));
    insert->operacion = INSERT;
    char **listSplit = string_split(comando, "\"");
    char **listSplit2;
    if (listSplit[1] == NULL) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        free(insert);
        return NULL;
    }
    insert->value = strdup(listSplit[1]);
    if (listSplit[2]) {
        insert->timestamp = strtol(listSplit[2], &hayString, 10);
        if (!string_is_empty(hayString)) {
            string_iterate_lines(listSplit, (void *) free);
            free(listSplit);

            free(insert);
            return NULL;
        }
    } else {
        insert->timestamp = obtenerMilisegundosDeHoy();
    }

    listSplit2 = string_split(listSplit[0], " ");
    if (listSplit2[1] == NULL) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        free(insert);
        return NULL;
    }

    insert->nameTable = strdup(listSplit2[1]);
    if (listSplit2[2] == NULL) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        string_iterate_lines(listSplit2, (void *) free);
        free(listSplit2);

        free(insert);
        return NULL;
    }
    insert->key = strtol(listSplit2[2], &hayString, 10);
    if (!string_is_empty(hayString)) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        string_iterate_lines(listSplit2, (void *) free);
        free(listSplit2);
        free(insert);
        return NULL;
    }

    string_iterate_lines(listSplit, (void *) free);
    free(listSplit);

    string_iterate_lines(listSplit2, (void *) free);
    free(listSplit2);
    return insert;
}

st_select *cargarSelect(char *comando) {
    char *hayString;
    st_select *_select = malloc(sizeof(st_select));
    _select->operacion = SELECT;
    char **listSplit = string_split(comando, " ");
    if (listSplit[1] == NULL) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        free(_select);
        return NULL;
    }
    _select->nameTable = strdup(listSplit[1]);
    if (listSplit[2] == NULL) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        free(_select);
        return NULL;
    }
    _select->key = strtol(listSplit[2], &hayString, 10);
    if (!string_is_empty(hayString)) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        free(_select);
        return NULL;
    }

    string_iterate_lines(listSplit, (void *) free);
    free(listSplit);
    return _select;
}

st_create *cargarCreate(char *comando) {
    char *hayString;
    st_create *_create = malloc(sizeof(st_create));
    _create->operacion = CREATE;
    char **listSplit = string_split(comando, " ");
    if (listSplit[1] == NULL) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        free(_create);
        return NULL;
    }
    _create->nameTable = strdup(listSplit[1]);
    if (listSplit[2] == NULL) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        free(_create);
        return NULL;
    }
    _create->tipoConsistencia = strdup(listSplit[2]);
    if (listSplit[3] == NULL) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        free(_create);
        return NULL;
    }
    _create->numeroParticiones = strtol(listSplit[3], &hayString, 10);
    if (!string_is_empty(hayString)) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        free(_create);
        return NULL;
    }
    if (listSplit[4] == NULL) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        free(_create);
        return NULL;
    }
    _create->compactionTime = strtol(listSplit[4], &hayString, 10);
    if (!string_is_empty(hayString)) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        free(_create);
        return NULL;
    }

    string_iterate_lines(listSplit, (void *) free);
    free(listSplit);
    return _create;

}

st_drop *cargarDrop(char *comando) {
    st_drop *_drop = malloc(sizeof(st_drop));
    _drop->operacion = DROP;
    char **listSplit = string_split(comando, " ");
    if (listSplit[1] == NULL) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        free(_drop);
        return NULL;
    }
    _drop->nameTable = strdup(listSplit[1]);

    string_iterate_lines(listSplit, (void *) free);
    free(listSplit);
    return _drop;
}

void *serealizarInsert(st_insert *insert, size_t *length) {
    st_size_insert sizeStruct;
    sizeStruct.size_operacion = sizeof(insert->operacion);
    sizeStruct.size_nametable = strlen(insert->nameTable) + 1;
    sizeStruct.size_key = sizeof(insert->key);
    sizeStruct.size_value = strlen(insert->value) + 1;
    sizeStruct.size_timestamp = sizeof(insert->timestamp);
    size_t sizeBuffer =
            sizeof(sizeStruct) + sizeStruct.size_operacion + sizeStruct.size_nametable + sizeStruct.size_key +
            sizeStruct.size_value + sizeStruct.size_timestamp;
    void *buffer = malloc(sizeBuffer);
    int offset = 0;
    memcpy(buffer, &sizeStruct, sizeof(sizeStruct));
    offset += sizeof(sizeStruct);

    memcpy((buffer + offset), &insert->operacion, sizeStruct.size_operacion);
    offset += sizeStruct.size_operacion;

    memcpy((buffer + offset), insert->nameTable, sizeStruct.size_nametable);
    offset += sizeStruct.size_nametable;

    memcpy((buffer + offset), &insert->key, sizeStruct.size_key);
    offset += sizeStruct.size_key;

    memcpy((buffer + offset), insert->value, sizeStruct.size_value);
    offset += sizeStruct.size_value;

    memcpy((buffer + offset), &insert->timestamp, sizeStruct.size_timestamp);
    *length = sizeBuffer;
    return buffer;
}

st_insert *desserealizarInsert(void *buffer) {
    st_size_insert sizeStruct;
    st_insert *insert = malloc(sizeof(st_insert));
    int offset = 0;

    memcpy(&sizeStruct, (buffer + offset), sizeof(sizeStruct));
    offset += sizeof(sizeStruct);

    memcpy(&insert->operacion, (buffer + offset), sizeStruct.size_operacion);
    offset += sizeStruct.size_operacion;

    insert->nameTable = malloc(sizeStruct.size_nametable);
    memcpy(insert->nameTable, (buffer + offset), sizeStruct.size_nametable);
    offset += sizeStruct.size_nametable;

    memcpy(&insert->key, (buffer + offset), sizeStruct.size_key);
    offset += sizeStruct.size_key;

    insert->value = malloc(sizeStruct.size_value);
    memcpy(insert->value, (buffer + offset), sizeStruct.size_value);
    offset += sizeStruct.size_value;

    memcpy(&insert->timestamp, (buffer + offset), sizeStruct.size_timestamp);

    return insert;
}

void destroyInsert(st_insert *insert) {
    free(insert->nameTable);
    free(insert->value);
    free(insert);
}

int getEnumFromString(char *string) {
    static struct {
        char *s;
        enum OPERACION e;
    } map[] = {
            {"INSERT", INSERT},
            {"SELECT", SELECT},
            {"CREATE", CREATE},
            {"DROP",   DROP},
			{"DESCRIBE", DESCRIBE},
			{"EXIT", EXIT},
			{"JOURNAL", JOURNAL}
    };

    for (unsigned int i = 0; i < sizeof(map) / sizeof(map[0]); i++) {

        if (string_starts_with(string, map[i].s)) {
            return map[i].e;
        }
    }
    return -1;
}

void destroySplit(char **split) {
    char *palabra;
    int i = 1;
    palabra = split[0];
    while (palabra) {
        free(palabra);
        palabra = split[i];
        ++i;
    }
    free(split);
}

//si devuelve NULL es un Describe global
st_describe *cargarDescribe(char *comando) {
    st_describe *describe = malloc(sizeof(st_describe));
    char **listSplit = string_split(comando, " ");
    if (listSplit[1] == NULL) {
        destroySplit(listSplit);
        free(describe);
        return NULL;
    }
    describe->operacion = DESCRIBE;
    describe->nameTable = strdup(listSplit[1]);
    destroySplit(listSplit);
    return describe;
}

void destroyDescribe(st_describe *describe) {
    free(describe->nameTable);
    free(describe);
}

void *serealizarDescribe(st_describe *describe, size_t *size_buffer) {
    st_size_describe sizeDescribe;
    sizeDescribe.operacion = sizeof(enum OPERACION);
    sizeDescribe.nameTable = strlen(describe->nameTable) + 1;

    *size_buffer = sizeof(st_size_describe) + sizeDescribe.operacion + sizeDescribe.nameTable;
    void *buffer = malloc(*size_buffer);
    int offset = 0;

    memcpy(buffer, &sizeDescribe, sizeof(st_size_describe));
    offset += sizeof(st_size_describe);

    memcpy((buffer + offset), &describe->operacion, sizeDescribe.operacion);
    offset += sizeDescribe.operacion;

    memcpy((buffer + offset), describe->nameTable, sizeDescribe.nameTable);

    return buffer;
}

st_describe *deserealizarDescribe(void *buffer) {
    st_size_describe sizeDescribe;
    st_describe *describe = malloc(sizeof(st_describe));
    int offset = 0;

    memcpy(&sizeDescribe, buffer, sizeof(sizeDescribe));
    offset += sizeof(sizeDescribe);

    memcpy(&describe->operacion, (buffer + offset), sizeDescribe.operacion);
    offset += sizeDescribe.operacion;

    describe->nameTable = malloc(sizeDescribe.nameTable);
    memcpy(describe->nameTable, (buffer + offset), sizeDescribe.nameTable);

    return describe;
}

void destroyCreate(st_create *create) {
    free(create->nameTable);
    free(create->tipoConsistencia);
    free(create);
}

void *serealizarCreate(st_create *create, size_t *size_buffer) {
    st_size_create sizeCreate;
    sizeCreate.operacion = sizeof(enum OPERACION);
    sizeCreate.nameTable = strlen(create->nameTable) + 1;
    sizeCreate.tipoConsistencia = strlen(create->tipoConsistencia) + 1;
    sizeCreate.numeroParticiones = sizeof(create->numeroParticiones);
    sizeCreate.compactionTime = sizeof(create->compactionTime);

    *size_buffer = sizeof(sizeCreate) + sizeCreate.operacion + sizeCreate.nameTable + sizeCreate.tipoConsistencia +
                   sizeCreate.numeroParticiones + sizeCreate.compactionTime;
    void *buffer = malloc(*size_buffer);
    int offset = 0;

    memcpy(buffer, &sizeCreate, sizeof(sizeCreate));
    offset += sizeof(sizeCreate);

    memcpy((buffer + offset), &create->operacion, sizeCreate.operacion);
    offset += sizeCreate.operacion;

    memcpy((buffer + offset), create->nameTable, sizeCreate.nameTable);
    offset += sizeCreate.nameTable;

    memcpy((buffer + offset), create->tipoConsistencia, sizeCreate.tipoConsistencia);
    offset += sizeCreate.tipoConsistencia;

    memcpy((buffer + offset), &create->numeroParticiones, sizeCreate.numeroParticiones);
    offset += sizeCreate.numeroParticiones;

    memcpy((buffer + offset), &create->compactionTime, sizeCreate.compactionTime);

    return buffer;
}

st_create *deserealizarCreate(void *buffer) {
    st_size_create sizeCreate;
    st_create *create = malloc(sizeof(st_create));
    int offset = 0;

    memcpy(&sizeCreate, buffer, sizeof(sizeCreate));
    offset += sizeof(sizeCreate);

    memcpy(&create->operacion, (buffer + offset), sizeCreate.operacion);
    offset += sizeCreate.operacion;

    create->nameTable = malloc(sizeCreate.nameTable);
    memcpy(create->nameTable, (buffer + offset), sizeCreate.nameTable);
    offset += sizeCreate.nameTable;

    create->tipoConsistencia = malloc(sizeCreate.tipoConsistencia);
    memcpy(create->tipoConsistencia, (buffer + offset), sizeCreate.tipoConsistencia);
    offset += sizeCreate.tipoConsistencia;

    memcpy(&create->numeroParticiones, (buffer + offset), sizeCreate.numeroParticiones);
    offset += sizeCreate.numeroParticiones;

    memcpy(&create->compactionTime, (buffer + offset), sizeCreate.compactionTime);

    return create;
}

void destoySelect(st_select *_select) {
    free(_select->nameTable);
    free(_select);
}

void *serealizarSelect(st_select *_select, size_t *size_buffer) {
    st_size_select sizeSelect;
    sizeSelect.operacion = sizeof(enum OPERACION);
    sizeSelect.nameTable = strlen(_select->nameTable) + 1;
    sizeSelect.key = sizeof(_select->key);

    *size_buffer = sizeof(st_size_select) + sizeSelect.operacion + sizeSelect.nameTable + sizeSelect.key;
    void *buffer = malloc(*size_buffer);
    int offset = 0;

    memcpy(buffer, &sizeSelect, sizeof(st_size_select));
    offset += sizeof(st_size_select);

    memcpy((buffer + offset), &_select->operacion, sizeSelect.operacion);
    offset += sizeSelect.operacion;

    memcpy((buffer + offset), _select->nameTable, sizeSelect.nameTable);
    offset += sizeSelect.nameTable;

    memcpy((buffer + offset), &_select->key, sizeSelect.key);

    return buffer;
}

st_select *deserealizarSelect(void *buffer) {
    st_size_select sizeSelect;
    st_select *_select = malloc(sizeof(st_select));
    int offset = 0;

    memcpy(&sizeSelect, buffer, sizeof(st_size_select));
    offset += sizeof(st_size_select);

    memcpy(&_select->operacion, (buffer + offset), sizeSelect.operacion);
    offset += sizeSelect.operacion;

    _select->nameTable = malloc(sizeSelect.nameTable);
    memcpy(_select->nameTable, (buffer + offset), sizeSelect.nameTable);
    offset += sizeSelect.nameTable;

    memcpy(&_select->key, (buffer + offset), sizeSelect.key);

    return _select;
}

void destroyDrop(st_drop *drop) {
    free(drop->nameTable);
    free(drop);
}

void *serealizarDrop(st_drop *drop, size_t *size_buffer) {
    st_size_drop sizeDrop;
    sizeDrop.operacion = sizeof(enum OPERACION);
    sizeDrop.nameTable = strlen(drop->nameTable) + 1;

    *size_buffer = sizeof(st_size_drop) + sizeDrop.operacion + sizeDrop.nameTable;
    void *buffer = malloc(*size_buffer);
    int offset = 0;

    memcpy(buffer, &sizeDrop, sizeof(st_size_drop));
    offset += sizeof(st_size_drop);

    memcpy((buffer + offset), &drop->operacion, sizeDrop.operacion);
    offset += sizeDrop.operacion;

    memcpy((buffer + offset), drop->nameTable, sizeDrop.nameTable);

    return buffer;
}

st_drop *deserealizarDrop(void *buffer) {
    st_size_drop sizeDrop;
    st_drop *drop = malloc(sizeof(st_drop));
    int offset = 0;

    memcpy(&sizeDrop, buffer, sizeof(st_drop));
    offset += sizeof(st_drop);

    memcpy(&drop->operacion, (buffer + offset), sizeDrop.operacion);
    offset += sizeDrop.operacion;

    drop->nameTable = malloc(sizeDrop.nameTable);
    memcpy(drop->nameTable, (buffer + offset), sizeDrop.nameTable);

    return drop;
}
