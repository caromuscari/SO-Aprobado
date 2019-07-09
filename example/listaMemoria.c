#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <funcionesCompartidas/listaMemoria.h>

st_data_memoria *cargarListMemoria() {
    st_data_memoria * dataMemoria = malloc(sizeof(st_data_memoria));
    dataMemoria->numero = 1;
    dataMemoria->listaMemorias = list_create();

    st_memoria *memoria = malloc(sizeof(st_memoria));
    memoria->ip = strdup("127.0.0.1");
    memoria->puerto = strdup("3001");
    memoria->numero = 2;
    list_add(dataMemoria->listaMemorias, memoria);

    st_memoria *memoria2 = malloc(sizeof(st_memoria));
    memoria2->ip = strdup("3002");
    memoria2->puerto = strdup("127.0.0.1.5.6");
    memoria2->numero = 3;
    list_add(dataMemoria->listaMemorias, memoria2);

    return dataMemoria;
}

int main(int argc, char **argv) {
    st_data_memoria * dataMemoria = cargarListMemoria();
    size_t sizeBuffer = 0;
    void *buffer = serealizarMemoria(dataMemoria, &sizeBuffer);
    st_data_memoria *ok = deserealizarMemoria(buffer, sizeBuffer);
    free(buffer);
    printf("%d\n",dataMemoria->numero);
    for (int i = 0; i < ok->listaMemorias->elements_count; i++) {
        st_memoria *memoria = list_get(ok->listaMemorias, i);
        printf(" ip = %s\n", memoria->ip);
        printf(" puerto = %s\n", memoria->puerto);
        printf(" numero = %d\n", memoria->numero);
    }
    destroyListaDataMemoria(dataMemoria);
    destroyListaDataMemoria(ok);
}