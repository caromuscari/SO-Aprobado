//
// Created by miguelchauca on 28/04/19.
//

#include "threadPoolMemoria.h"

t_list *poolMemoria;
extern config *configuracion;
extern t_log *file_log;
pthread_mutex_t mutex;

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


bool existeTipoDeCriterio(t_list *criterios, TypeCriterio tipo) {
    int i;
    TypeCriterio *criterio;
    for (i = 0; i < criterios->elements_count; ++i) {
        criterio = list_get(criterios, i);
        if (*criterio == tipo) {
            return true;
        }
    }
    return false;
}

bool existeTag(t_list *tags, char *text) {
    int i;
    char *tag;
    for (i = 0; i < tags->elements_count; ++i) {
        tag = list_get(tags, i);
        if (strcmp(tag, text) == 0) {
            return true;
        }
    }
    return false;
}

st_memoria * getMemoriaSCHSC(char *text, TypeCriterio tipo) {
    int i;
    st_kernel_memoria *memoria;
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        memoria = list_get(poolMemoria, i);
        if (memoria->activo && existeTipoDeCriterio(memoria->tipos, tipo)) {
            if (existeTag(memoria->tags, text)) {
                return memoria->memoria;
            }
        }
    }
    //buscar que memoria esta disponible tomamos al primero como posible minimo
    int posicion = -1;
    int countMinimo = 0;
    bool flagFirst = true;
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        memoria = list_get(poolMemoria, i);
        if (memoria->activo && existeTipoDeCriterio(memoria->tipos, tipo)) {
            if (flagFirst) {
                countMinimo = memoria->count;
                posicion = i;
                flagFirst = false;
            } else {
                if (memoria->count < countMinimo) {
                    countMinimo = memoria->count;
                    posicion = i;
                }
            }
        }
    }
    //cargamos un nuevo tag a la memoria elegida
    if(posicion != -1){
        memoria = list_get(poolMemoria, posicion);
        list_add(memoria->tags,strdup(text));
        return memoria->memoria;
    }
    return NULL;
}

st_memoria * getMemoriaEventual(){
    int i;
    st_kernel_memoria *memoria;
    int pos = -1;
    int mini = 0;
    bool flagFisrt = true;
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        memoria = list_get(poolMemoria, i);
        if (memoria->activo && existeTipoDeCriterio(memoria->tipos, EventualConsistency)) {
            if(flagFisrt){
                pos = i;
                mini = memoria->count;
                flagFisrt = false;
            }else{
                if(memoria->count < mini){
                    pos = i;
                    mini = memoria->count;
                }
            }
        }
    }
    if(pos != -1){
        memoria = list_get(poolMemoria, pos);
        return memoria->memoria;
    }

    return NULL;
}

st_memoria * getMemoriaRandom(){
    int i;
    st_kernel_memoria *memoria;
    int pos = -1;
    int mini = 0;
    bool flagFisrt = true;
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        memoria = list_get(poolMemoria, i);
        if(flagFisrt){
            pos = i;
            mini = memoria->count;
            flagFisrt = false;
        }else{
            if(memoria->count < mini){
                pos = i;
                mini = memoria->count;
            }
        }
    }
    if(pos != -1){
        memoria = list_get(poolMemoria, pos);
        return memoria->memoria;
    }

    return NULL;
}

st_memoria *getMemoria(TypeCriterio tipo, char *text) {
    switch (tipo) {
        case StrongConsistency: {
            return getMemoriaSCHSC(text, StrongConsistency);
        }
        case StrongHashConsistency: {
            return getMemoriaSCHSC(text, StrongHashConsistency);
        }
        case EventualConsistency: {
            return getMemoriaEventual();
        }
        default: {
            return getMemoriaRandom();
        }
    }
}

st_kernel_memoria *cargarNuevaKernelMemoria(st_memoria *data) {
    st_kernel_memoria *k_memoria = malloc(sizeof(st_kernel_memoria));
    st_memoria *memoria = malloc(sizeof(st_data_memoria));

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

st_kernel_memoria *existeMemoria(int numeroMemoria) {
    int i;
    st_kernel_memoria *memoria;
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        memoria = list_get(poolMemoria, i);
        if (memoria->memoria->numero == numeroMemoria) {
            return memoria;
        }
    }
    return NULL;
}

bool setTipoConsistencia(int number, TypeCriterio tipo) {
    bool flagSolucion = false;
    TypeCriterio *auxTipo;
    st_kernel_memoria *memoria = existeMemoria(number);
    if (memoria && memoria->activo) {
        flagSolucion = true;
        auxTipo = malloc(sizeof(TypeCriterio));
        *auxTipo = tipo;
        list_add(memoria->tipos, auxTipo);
    } else {
        log_info(file_log, "no se encontro esa memoria");
    }
    return flagSolucion;
}

void agregarMemoria(st_kernel_memoria *kernelMemoria) {
    list_add(poolMemoria, kernelMemoria);
}

void updateMemoria(st_kernel_memoria *kernelMemoria, st_memoria *stMemoria) {
    destroyMemoria(kernelMemoria->memoria);
    st_memoria *memoria = malloc(sizeof(st_data_memoria));
    memoria->numero = stMemoria->numero;
    memoria->puerto = strdup(stMemoria->puerto);
    memoria->ip = strdup(stMemoria->ip);
    kernelMemoria->memoria = memoria;
    kernelMemoria->activo = true;
}

void updateListaMemorias(st_data_memoria *dataMemoria) {
    //crear la memoria que consulto
    st_kernel_memoria *newKernelMemoria;
    st_memoria *stMemoria;
    newKernelMemoria = existeMemoria(dataMemoria->numero);
    if (newKernelMemoria == NULL) {
        //agregar
        stMemoria = malloc(sizeof(st_memoria));
        stMemoria->numero = dataMemoria->numero;
        stMemoria->puerto = strdup(configuracion->PUERTO_MEMORIA);
        stMemoria->ip = strdup(configuracion->IP_MEMORIA);
        newKernelMemoria = cargarNuevaKernelMemoria(stMemoria);
        agregarMemoria(newKernelMemoria);
        destroyMemoria(stMemoria);
    } else {
        //update
        newKernelMemoria->memoria->numero = dataMemoria->numero;
        newKernelMemoria->activo = true;
    }
    int i;
    for (i = 0; i < dataMemoria->listaMemorias->elements_count; ++i) {
        stMemoria = list_get(dataMemoria->listaMemorias, i);
        newKernelMemoria = existeMemoria(stMemoria->numero);
        if (newKernelMemoria == NULL) {
            //adding
            newKernelMemoria = cargarNuevaKernelMemoria(stMemoria);
            agregarMemoria(newKernelMemoria);
        } else {
            //update
            updateMemoria(newKernelMemoria, stMemoria);
        }
        destroyMemoria(stMemoria);

    }
}

void destoyPoolMemoria() {
    int i;
    st_kernel_memoria *memoria;
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        memoria = list_get(poolMemoria, i);
        destroyMemoria(memoria->memoria);
        list_destroy(memoria->tags);
        free(memoria);
    }
    list_destroy(poolMemoria);
}

void CleanListaMemoria() {
    int i;
    st_kernel_memoria *kernelMemoria;
    for (i = 0; i < poolMemoria->elements_count; ++i) {
        kernelMemoria = list_get(poolMemoria, i);
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
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("\n mutex init failed\n");
        return NULL;
    }
    while (1) {
        CleanListaMemoria();
        control = 0;
        socketClient = establecerConexion(configuracion->IP_MEMORIA, configuracion->PUERTO_MEMORIA, file_log, &control);
        if (socketClient != -1) {
            request.letra = 'K';
            request.codigo = BUSCARTABLAGOSSIPING;
            request.sizeData = 0;
            bufferMensaje = createMessage(&request, NULL);
            enviar_message(socketClient, bufferMensaje, file_log, &control);
            if (bufferMensaje->buffer) free(bufferMensaje->buffer);
            free(bufferMensaje);
            if (control == 0) {
                buffer = getMessage(socketClient, &response, &control);
                if (control >= 0) {
                    updateListaMemorias(deserealizarMemoria(buffer, response.sizeData));
                    if (buffer) free(buffer);
                }
            }
            close(socketClient);
        }
        sleep(500);
    }
}