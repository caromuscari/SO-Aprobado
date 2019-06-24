//
// Created by miguelchauca on 23/06/19.
//

#include "registroTabla.h"

int tamanioDeSplit(char **split) {
    int count = 1;
    char *word = split[(count - 1)];
    while (word != NULL) {
        word = split[count];
        count++;
    }
    return count - 1;
}

st_registro *cargarRegistro(char *text) {
    st_registro *registro = malloc(sizeof(st_registro));
    char **splitText = string_split(text, ";");
    int cantidadPalabras = tamanioDeSplit(splitText);
    if (cantidadPalabras == 3) {
        registro->timestamp = atoi(splitText[0]);
        registro->key = atoi(splitText[1]);
        registro->value = strdup(splitText[2]);
        string_iterate_lines(splitText, (void *) free);
        free(splitText);
    } else {
        string_iterate_lines(splitText, (void *) free);
        free(registro);
        registro = NULL;
    }
    return registro;
}

void *serealizarRegistro(st_registro *registro, size_t *sizeBUffer) {
    st_size_registro sizeRegistro;
    sizeRegistro.timestamp = sizeof(registro->timestamp);
    sizeRegistro.key = sizeof(registro->key);
    sizeRegistro.value = strlen(registro->value) + 1;

    *sizeBUffer = sizeRegistro.timestamp + sizeRegistro.key + sizeRegistro.value + sizeof(st_size_registro);
    void *buffer = malloc(*sizeBUffer);
    int offset = 0;

    memcpy((buffer + offset), &sizeRegistro, sizeof(st_size_registro));
    offset += sizeof(st_size_registro);

    memcpy((buffer + offset), &registro->timestamp, sizeRegistro.timestamp);
    offset += sizeRegistro.timestamp;

    memcpy((buffer + offset), &registro->key, sizeRegistro.key);
    offset += sizeRegistro.key;

    memcpy((buffer + offset), registro->value, sizeRegistro.value);

    return buffer;
}

st_registro *deserealizarRegistro(void *buffer) {
    st_size_registro sizeRegistro;
    st_registro *registro = malloc(sizeof(st_registro));
    int offset = 0;

    memcpy(&sizeRegistro, buffer, sizeof(st_size_registro));
    offset += sizeof(st_size_registro);

    memcpy(&registro->timestamp, (buffer + offset), sizeRegistro.timestamp);
    offset += sizeRegistro.timestamp;

    memcpy(&registro->key, (buffer + offset), sizeRegistro.key);
    offset += sizeRegistro.key;

    registro->value = malloc(sizeRegistro.value);
    memcpy(registro->value, (buffer + offset), sizeRegistro.value);

    return registro;
}

void destroyRegistro(st_registro *registro) {
    free(registro->value);
    free(registro);
}