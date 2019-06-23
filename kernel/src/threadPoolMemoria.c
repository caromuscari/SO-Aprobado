//
// Created by miguelchauca on 28/04/19.
//

#include "threadPoolMemoria.h"

t_list *poolMemoria;
extern config *configuracion;
extern t_log *file_log;
pthread_mutex_t mutex;

void show(){
    st_kernel_memoria * memoria;
    int i,j;
    enum TypeCriterio * tipo;
    for (i = 0; i < poolMemoria->elements_count ; ++i) {
        memoria = list_get(poolMemoria,i);
        printf("ip = %s\n",memoria->memoria->ip);
        printf("puerto = %s\n",memoria->memoria->puerto);
        printf("numero = %d\n",memoria->memoria->numero);
        printf("activo = %d\n",memoria->activo);
        for (j = 0; j < memoria->tipos->elements_count; ++j) {
            tipo = list_get(memoria->tipos,j);
            printf("tipo = %d\n",*tipo);
        }
        printf("------------------- ---------------------------\n\n");
    }
}

bool tieneElTipo(t_list * listaTipos, enum TypeCriterio tipo){
    int search_tipo(enum TypeCriterio *p) {
        if(*p == tipo){
            return true;
        }
        return false;
    }
    return list_find(listaTipos, (void *) search_tipo);
}

st_data_memoria * getSCMemoria(char * nameTable){
    int i,j;
    st_kernel_memoria * memoria;
    char * tag;
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        memoria = list_get(poolMemoria,i);
        if(memoria->activo && tieneElTipo(memoria->tipos,StrongConsistency)){
            for (j = 0; j < memoria->tags->elements_count ; ++j) {
                tag = list_get(memoria->tags,j);
                if(strcmp(tag,nameTable) == 0){
                    return memoria->memoria;
                }
            }
        }
    }
    return NULL;
}

st_data_memoria * getMemoria(enum TypeCriterio tipoConsistencia, char * text){
    switch (tipoConsistencia){
        case StrongConsistency:{
            return getSCMemoria(text);
        }
        case StrongHashConsistency:{
            break;
        }
        default:{

            break;
        }
    }

}

st_kernel_memoria * getMemoriaKernelByNumber(int numMemoria){
    void * result;
    int search_Memoria(st_kernel_memoria *p) {
        if(p->memoria->numero == numMemoria){
            return true;
        }
        return false;
    }
    list_find(poolMemoria, (void *) search_Memoria);
}

st_data_memoria * getMemoriaByNumber(int numMemoria, t_list * listaMemorias){
    void * result;
    int search_Memoria(st_data_memoria *p) {
        if(p->numero == numMemoria){
            return true;
        }
        return false;
    }
    return list_find(listaMemorias, (void *) search_Memoria);
}

bool setTipoConsistencia(int number, enum TypeCriterio tipo){
    bool flagSolucion = false;
    enum TypeCriterio * auxTipo;
    st_kernel_memoria * memoria = getMemoriaKernelByNumber(number);
    if(memoria){
        flagSolucion = true;
        auxTipo = malloc(sizeof(enum TypeCriterio));
        *auxTipo = tipo;
        list_add(memoria->tipos,auxTipo);
    }
    return  flagSolucion;
}

st_kernel_memoria * cargarNuevaKernelMemoria(st_data_memoria * data){
    st_kernel_memoria * k_memoria = malloc(sizeof(st_kernel_memoria));
    st_data_memoria * memoria = malloc(sizeof(st_data_memoria));

    memoria->numero = data->numero;
    memoria->puerto = strdup(data->puerto);
    memoria->ip = strdup(data->ip);

    k_memoria->tipos = list_create();
    k_memoria->activo = true;
    k_memoria->tags = list_create();
    k_memoria->memoria = memoria;
    k_memoria->count = 0;

    return k_memoria;
}

void updateListaMemorias(t_list * nuevaLista){
    int i;
    st_data_memoria * memoria;
    st_kernel_memoria * k_memoria;
    if(!poolMemoria){
        poolMemoria = list_create();
    }
    //agregamos nuevas memorias
    for (i = 0; i < nuevaLista->elements_count; ++i) {
        memoria = list_get(nuevaLista,i);
        if(!getMemoriaKernelByNumber(memoria->numero)){
            k_memoria = cargarNuevaKernelMemoria(memoria);
//            pthread_mutex_lock(&mutex);
            list_add(poolMemoria,k_memoria);
            //pthread_mutex_unlock(&mutex);
        }
    }
    //actualizamos los estado de las memorias
    for (i = 0; i < poolMemoria->elements_count; ++i){
        //pthread_mutex_lock(&mutex);
        k_memoria = list_get(poolMemoria,i);
        if(!getMemoriaByNumber(k_memoria->memoria->numero,nuevaLista)){
            k_memoria->activo = false;
        }
        //pthread_mutex_unlock(&mutex);
    }
    show();
    destroyListaDataMemoria(nuevaLista);
}

void destoyPoolMemoria(){
    int i;
    st_kernel_memoria * memoria;
    for (i = 0; i < poolMemoria->elements_count ; ++i) {
        memoria = list_get(poolMemoria,i);
        destroyMemoria(memoria->memoria);
        list_destroy(memoria->tags);
        free(memoria);
    }
    list_destroy(poolMemoria);
}

void CleanListaMemoria(){
    pthread_mutex_lock(&mutex);
    if(poolMemoria != NULL){
        destoyPoolMemoria(poolMemoria);
    }
    poolMemoria = list_create();
    pthread_mutex_unlock(&mutex);
}

void *loadPoolMemori() {
    //conectarse con la memoria y pelirle la lista
    poolMemoria = list_create();
    int socketClient;
    int control = 0;
    message *bufferMensaje;
    header request;
    header response;
    void *buffer = NULL;
    bool error;
    poolMemoria = NULL;
    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return NULL;
    }
    while (1) {
        error = true;
        control = 0;
        socketClient = establecerConexion(configuracion->IP_MEMORIA, configuracion->PUERTO_MEMORIA, file_log, &control);
        if (socketClient != -1) {
            request.letra = 'K';
            request.codigo = 5;
            request.sizeData = 1;
            bufferMensaje = createMessage(&request, " ");
            enviar_message(socketClient, bufferMensaje, file_log, &control);
            free(bufferMensaje);
            if (control == 0) {
                buffer = getMessage(socketClient, &response, &control);
                if (buffer) {
                    error = false;
                    updateListaMemorias(deserealizarListaDataMemoria(buffer, response.sizeData));
                    close(socketClient);
                    free(buffer);
                }
            }
        }
        if (error) {
            CleanListaMemoria();
        }
        sleep(300);
    }
}