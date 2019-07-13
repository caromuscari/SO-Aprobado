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

//bool tieneElTipo(t_list * listaTipos, enum TypeCriterio tipo){
//    int search_tipo(enum TypeCriterio *p) {
//        if(*p == tipo){
//            return true;
//        }
//        return false;
//    }
//    return list_find(listaTipos, (void *) search_tipo);
//}
//
//st_memoria * getSCMemoria(char * nameTable){
//    int i,j;
//    st_kernel_memoria * memoria;
//    char * tag;
//    for (i = 0; i < poolMemoria->elements_count; ++i) {
//        memoria = list_get(poolMemoria,i);
//        if(memoria->activo && tieneElTipo(memoria->tipos,StrongConsistency)){
//            for (j = 0; j < memoria->tags->elements_count ; ++j) {
//                tag = list_get(memoria->tags,j);
//                if(strcmp(tag,nameTable) == 0){
//                    return memoria->memoria;
//                }
//            }
//        }
//    }
//    return NULL;
//}
//
//st_memoria * getMemoria(enum TypeCriterio tipoConsistencia, char * text){
//    switch (tipoConsistencia){
//        case StrongConsistency:{
//            return getSCMemoria(text);
//        }
//        case StrongHashConsistency:{
//            break;
//        }
//        default:{
//
//            break;
//        }
//    }
//
//}
//
//st_kernel_memoria * getMemoriaKernelByNumber(int numMemoria){
//    void * result;
//    int search_Memoria(st_kernel_memoria *p) {
//        if(p->memoria->numero == numMemoria){
//            return true;
//        }
//        return false;
//    }
//    list_find(poolMemoria, (void *) search_Memoria);
//}
//
//st_data_memoria * getMemoriaByNumber(int numMemoria, t_list * listaMemorias){
//    void * result;
//    int search_Memoria(st_data_memoria *p) {
//        if(p->numero == numMemoria){
//            return true;
//        }
//        return false;
//    }
//    return list_find(listaMemorias, (void *) search_Memoria);
//}
//

st_kernel_memoria * cargarNuevaKernelMemoria(st_memoria * data){
    st_kernel_memoria * k_memoria = malloc(sizeof(st_kernel_memoria));
    st_memoria * memoria = malloc(sizeof(st_data_memoria));

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

st_kernel_memoria * existeMemoria(int numeroMemoria){
    int i;
    st_kernel_memoria * memoria;
    for (i = 0; i < poolMemoria->elements_count ; ++i) {
        memoria = list_get(poolMemoria,i);
        if(memoria->memoria->numero == numeroMemoria){
            return memoria;
        }
    }
    return NULL;
}

bool setTipoConsistencia(int number, enum TypeCriterio tipo){
    bool flagSolucion = false;
    enum TypeCriterio * auxTipo;
    st_kernel_memoria * memoria = existeMemoria(number);
    if(memoria){
        flagSolucion = true;
        auxTipo = malloc(sizeof(enum TypeCriterio));
        *auxTipo = tipo;
        list_add(memoria->tipos,auxTipo);
    }
    return  flagSolucion;
}

void agregarMemoria(st_kernel_memoria * kernelMemoria){
    list_add(poolMemoria,kernelMemoria);
}

void updateMemoria(st_kernel_memoria * kernelMemoria, st_memoria * stMemoria){
    destroyMemoria(kernelMemoria->memoria);
    st_memoria * memoria = malloc(sizeof(st_data_memoria));
    memoria->numero = stMemoria->numero;
    memoria->puerto = strdup(stMemoria->puerto);
    memoria->ip = strdup(stMemoria->ip);
    kernelMemoria->memoria = memoria;
    kernelMemoria->activo = true;
}

void updateListaMemorias(st_data_memoria * dataMemoria){
    //crear la memoria que consulto
    st_kernel_memoria * newKernelMemoria;
    st_memoria * stMemoria;
    newKernelMemoria = existeMemoria(dataMemoria->numero);
    if(newKernelMemoria == NULL){
        //agregar
        stMemoria = malloc(sizeof(st_memoria));
        stMemoria->numero = dataMemoria->numero;
        stMemoria->puerto = strdup(configuracion->PUERTO_MEMORIA);
        stMemoria->ip = strdup(configuracion->IP_MEMORIA);
        newKernelMemoria = cargarNuevaKernelMemoria(stMemoria);
        agregarMemoria(newKernelMemoria);
        destroyMemoria(stMemoria);
    }else{
        //update
        newKernelMemoria->memoria->numero = dataMemoria->numero;
        newKernelMemoria->activo = true;
    }
    int i;
    for (i = 0; i < dataMemoria->listaMemorias->elements_count ; ++i) {
        stMemoria = list_get(dataMemoria->listaMemorias, i);
        newKernelMemoria = existeMemoria(stMemoria->numero);
        if(newKernelMemoria == NULL){
            //adding
            newKernelMemoria = cargarNuevaKernelMemoria(stMemoria);
            agregarMemoria(newKernelMemoria);
        }else{
            //update
            updateMemoria(newKernelMemoria,stMemoria);
        }
        destroyMemoria(stMemoria);

    }
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
    int i;
    st_kernel_memoria * kernelMemoria;
    for (i = 0; i < poolMemoria->elements_count ; ++i) {
        kernelMemoria = list_get(poolMemoria,i);
        kernelMemoria->activo = false;
    }
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
    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return NULL;
    }
    while (1) {
        CleanListaMemoria();
        control = 0;
        socketClient = establecerConexion(configuracion->IP_MEMORIA, configuracion->PUERTO_MEMORIA, file_log, &control);
        if (socketClient != -1) {
            request.letra = 'K';
            request.codigo = 8;
            request.sizeData = 1;
            bufferMensaje = createMessage(&request, " ");
            enviar_message(socketClient, bufferMensaje, file_log, &control);
            free(bufferMensaje);
            if (control == 0) {
                buffer = getMessage(socketClient, &response, &control);
                if (buffer) {
                    updateListaMemorias(deserealizarMemoria(buffer, response.sizeData));
                    close(socketClient);
                    free(buffer);
                }
            }
        }
        printf("********************************************************************************************\n");
        show();
        sleep(30);
    }
}