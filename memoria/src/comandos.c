#include "comandos.h"
#include "hiloInotify.h"

extern t_log *file_log;
extern int tamanioValue;
extern t_list* listaDeMarcos;
extern void *memoriaPrincipal;
extern t_list* listaDeSegmentos;
extern pthread_mutex_t mutexMemPrinc;

//void statusSegmento(){
//	st_segmento * segmento = NULL;
//	for(int i = 0; i < listaDeSegmentos->elements_count; i++){
//		segmento = list_get(listaDeSegmentos,i);
//		printf("segmento [%s] cantidad de paginas [%d]",segmento->nombreTabla, segmento->tablaDePaginas->elements_count);
//	}
//}

//COMANDO INSERT
int comandoInsert(st_insert* comandoInsert){
    if(strlen(comandoInsert->value) > tamanioValue){
        log_error(file_log, "[proceso Insert]El value es mayor al tamaño máximo");
        return MAYORQUEVALUEMAX;
    }
    log_info(file_log,"[proceso Insert] Iniciando proceso");
    pthread_mutex_lock(&mutexMemPrinc);
    st_segmento* segmentoEncontrado = buscarSegmentoPorNombreTabla(comandoInsert->nameTable);//devuelve el segmento con ese nombre de tabla
    if(segmentoEncontrado){
        log_info(file_log, "[proceso Insert]Segmento encontrado");
        st_tablaDePaginas* paginaDeTablaEncontrada = buscarPaginaPorKey(segmentoEncontrado->tablaDePaginas, comandoInsert->key);
        if(paginaDeTablaEncontrada){
            log_info(file_log, "[proceso Insert]Pagina encontrada");
            log_info(file_log, "[proceso Insert]Actualizando pagina");
            memcpy(paginaDeTablaEncontrada->pagina + sizeof(double) + sizeof(uint16_t), comandoInsert->value, string_length(comandoInsert->value)+1);
            memcpy(paginaDeTablaEncontrada->pagina, &comandoInsert->timestamp, sizeof(double));
            paginaDeTablaEncontrada->flagModificado = 1;
            log_info(file_log,"[proceso Insert] Finalizar proceso");

            sleep(getRetardoMem()/1000);
            pthread_mutex_unlock(&mutexMemPrinc);
            return OK;
        }
        log_info(file_log, "[proceso Insert]No se encontro pagina");
        log_info(file_log, "[proceso Insert]Buscando Disponibilidad en la memoria");
        int posMarcoLibre = buscarMarcoLibre();
        if(posMarcoLibre == -1){
            log_info(file_log, "proceso Insert]Memoria full");
            log_info(file_log,"[proceso Insert] Finalizar proceso");

            pthread_mutex_unlock(&mutexMemPrinc);
            return FULLMEMORY;
        }
        log_info(file_log, "[proceso Insert]Cargando Nueva Pagina");
        void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));
        memcpy(paginaLibre, &comandoInsert->timestamp, sizeof(double));
        memcpy(paginaLibre + sizeof(double), &comandoInsert->key, sizeof(uint16_t));
        int largo_value = strlen(comandoInsert->value);
        comandoInsert->value[largo_value]='\0';
        memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), comandoInsert->value, string_length(comandoInsert->value)+1);

        st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
        paginaDeTabla->nroDePagina = posMarcoLibre;
        paginaDeTabla->pagina = paginaLibre;
        paginaDeTabla->flagModificado = 1;

        log_info(file_log, "[proceso Insert]Agregando nueva pagina al segmento");
        list_add(segmentoEncontrado->tablaDePaginas, paginaDeTabla);
        st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
        marco->condicion = OCUPADO;
        log_info(file_log,"[proceso Insert] Finalizar proceso");

        sleep(getRetardoMem()/1000);
        pthread_mutex_unlock(&mutexMemPrinc);
        return OK;
    }

    log_info(file_log, "[proceso Insert]No se encontro segmento");
    log_info(file_log, "[proceso Insert]Buscando Disponibilidad en la memoria");
    int posMarcoLibre = buscarMarcoLibre();
    if(posMarcoLibre == -1){
        log_info(file_log, "proceso Insert]Memoria full");
        log_info(file_log,"[proceso Insert] Finalizar proceso");

        pthread_mutex_unlock(&mutexMemPrinc);
        return FULLMEMORY;
    }
    log_info(file_log, "[proceso Insert]Creando Nuevo Segmento");
    st_segmento* segmentoNuevo= malloc(sizeof(st_segmento));
    segmentoNuevo->nombreTabla = strdup(comandoInsert->nameTable);
    segmentoNuevo->tablaDePaginas = list_create();
    log_info(file_log, "[proceso Insert]Cargando Nueva Pagina");
    void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));
    memcpy(paginaLibre, &comandoInsert->timestamp, sizeof(double));
    memcpy(paginaLibre + sizeof(double), &comandoInsert->key, sizeof(uint16_t));
    int largo_value = strlen(comandoInsert->value);
    comandoInsert->value[largo_value]='\0';
    memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), comandoInsert->value, string_length(comandoInsert->value)+1);
    st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
    paginaDeTabla->nroDePagina = posMarcoLibre;
    paginaDeTabla->pagina = paginaLibre;
    paginaDeTabla->flagModificado = 1;
    log_info(file_log, "[proceso Insert]Agregando nueva pagina al segmento");
    list_add(segmentoNuevo->tablaDePaginas, paginaDeTabla);
    log_info(file_log, "[proceso Insert]Agregando nueva segmento a la lista de segmentos");
    segmentoNuevo->nroSegmento = list_size(listaDeSegmentos);
    list_add_in_index(listaDeSegmentos, segmentoNuevo->nroSegmento, segmentoNuevo);
    st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
    marco->condicion = OCUPADO;
    log_info(file_log,"[proceso Insert] Finalizar proceso");

    sleep(getRetardoMem()/1000);
    pthread_mutex_unlock(&mutexMemPrinc);
    return OK;
}

// COMANDO SELECT
st_registro* comandoSelect(st_select* comandoSelect, enum_resultados* resultado){
    log_info(file_log,"[proceso Select] Iniciando Proceso");
    st_segmento* segmentoEncontrado;
    st_registro* registro;
    pthread_mutex_lock(&mutexMemPrinc);
    segmentoEncontrado = buscarSegmentoPorNombreTabla(comandoSelect->nameTable);//devuelve el segmento con ese nombre de tabla

    if(segmentoEncontrado){
        log_info(file_log, "[proceso Select] Segmento encontrado");
        st_tablaDePaginas* paginaDeTablaEncontrada = buscarPaginaPorKey(segmentoEncontrado->tablaDePaginas, comandoSelect->key);
        if(paginaDeTablaEncontrada){
            log_info(file_log, "[proceso Select] Se Encontro Pagina");
            registro = malloc(sizeof(st_registro));
            registro->value = malloc(tamanioValue);
            st_marco* marco = list_get(listaDeMarcos, paginaDeTablaEncontrada->nroDePagina);
            marco->timestamp = obtenerMilisegundosDeHoy();
            memcpy(&registro->timestamp, paginaDeTablaEncontrada->pagina, sizeof(double));
            memcpy(&registro->key, paginaDeTablaEncontrada->pagina+sizeof(double), sizeof(uint16_t));
            memcpy(registro->value, paginaDeTablaEncontrada->pagina+sizeof(double)+sizeof(uint16_t), tamanioValue);
            *resultado = OK;
            log_info(file_log,"[proceso Select] Finalizando Proceso");

            pthread_mutex_unlock(&mutexMemPrinc);
            sleep(getRetardoMem()/1000);
            return registro;
        }
        log_info(file_log, "[proceso Select] No se Encontro Pagina");
        log_info(file_log, "[proceso Select] Buscar dato al FileSystem");
        registro = obtenerSelect(comandoSelect);
        if(registro == NULL){
            log_info(file_log, "[proceso Select] No se encontro dato en el FileSystem");
            *resultado = NOOK;
            log_info(file_log,"[proceso Select] Finalizando Proceso");

            pthread_mutex_unlock(&mutexMemPrinc);
            return NULL;
        }
        log_info(file_log, "[proceso Select] Buscando Disponibilidad en la memoria");
        int posMarcoLibre = buscarMarcoLibre();

        if(posMarcoLibre == -1){
            log_info(file_log, "[proceso Select] Memoria full");
            *resultado = FULLMEMORY;
            log_info(file_log,"[proceso Select] Finalizando Proceso");

            pthread_mutex_unlock(&mutexMemPrinc);
            return NULL;
        }
        log_info(file_log, "[proceso Select] Cargando Nueva Pagina");
        void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));

        memcpy(paginaLibre, &registro->timestamp, sizeof(double));
        memcpy(paginaLibre + sizeof(double), &registro->key, sizeof(uint16_t));

        int largo_value = strlen(registro->value);
        registro->value[largo_value]='\0';
        largo_value += 1;
        memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), registro->value, largo_value);

        st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
        paginaDeTabla->nroDePagina = posMarcoLibre;
        paginaDeTabla->pagina = paginaLibre;
        //paginaDeTabla->desplazamiento = string_length(registro->value);
        paginaDeTabla->flagModificado = 0;
        log_info(file_log, "[proceso Select] Agregando la nueva pagina al segmento");
        list_add(segmentoEncontrado->tablaDePaginas, paginaDeTabla);

        st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
        marco->condicion = OCUPADO;
        marco->timestamp = obtenerMilisegundosDeHoy();

        *resultado = OK;
        log_info(file_log,"[proceso Select] Finalizando Proceso");

        pthread_mutex_unlock(&mutexMemPrinc);
        sleep(getRetardoMem()/1000);
        return registro;
    }
    log_info(file_log, "[proceso Select] Segmento no encontrado");
    log_info(file_log, "[proceso Select] Buscando Dato al FileSystem");
    registro = obtenerSelect(comandoSelect);

    if(registro == NULL){
        log_info(file_log, "[proceso Select] No se encontro dato en el FileSystem");
        *resultado = NOOK;
        log_info(file_log,"[proceso Select] Finalizando Proceso");

        pthread_mutex_unlock(&mutexMemPrinc);
        return NULL;
    }

    log_info(file_log, "[proceso Select] Buscando Disponibilidad en la memoria");
    int posMarcoLibre = buscarMarcoLibre();
    if(posMarcoLibre == -1){
        log_info(file_log, "[proces Select] Memoria Full");
        *resultado = FULLMEMORY;
        log_info(file_log,"[proceso Select] Finalizando Proceso");

        pthread_mutex_unlock(&mutexMemPrinc);
        return NULL;
    }
    log_info(file_log, "[proceso Select] Creando Nuevo segmento");
    st_segmento* segmentoNuevo= malloc(sizeof(st_segmento));
    segmentoNuevo->nombreTabla = strdup(comandoSelect->nameTable);
    segmentoNuevo->tablaDePaginas = list_create();

    log_info(file_log, "[proceso Select] Cargando Nueva Pagina");
    void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));
    memcpy(paginaLibre, &registro->timestamp, sizeof(double));
    memcpy(paginaLibre + sizeof(double), &registro->key, sizeof(uint16_t));

    int largo_value = strlen(registro->value);
    registro->value[largo_value]='\0';
    largo_value += 1;
    memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), registro->value, largo_value);

    st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
    paginaDeTabla->nroDePagina = posMarcoLibre;
    paginaDeTabla->pagina = paginaLibre;
    //paginaDeTabla->desplazamiento = string_length(registro->value);
    paginaDeTabla->flagModificado = 0;

    log_info(file_log, "[proceso Select] Agregando la nueva pagina al segmento");
    list_add(segmentoNuevo->tablaDePaginas, paginaDeTabla);
    log_info(file_log, "[proceso Select] Agregando la nueva segmento a la lista de segmentos");
    segmentoNuevo->nroSegmento = list_size(listaDeSegmentos);
    list_add_in_index(listaDeSegmentos, segmentoNuevo->nroSegmento, segmentoNuevo);
    st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
    marco->condicion = OCUPADO;
    marco->timestamp = obtenerMilisegundosDeHoy();

    *resultado = OK;
    log_info(file_log,"[proceso Select] Finalizando Proceso");

    pthread_mutex_unlock(&mutexMemPrinc);
    sleep(getRetardoMem()/1000);
    return registro;
}

void enviarPaginas(t_list * list_Paginas, char * nameTable){
    st_tablaDePaginas * auxPagina = NULL;
    st_insert * stInsert;
    st_marco * stMarco;
    t_list * list_paginas_fallidas = list_create();
    bool getSoloInsert(st_tablaDePaginas * stTablaDePaginas){
        return stTablaDePaginas->flagModificado == 1;
    }
    while (list_Paginas->elements_count > 0 && (auxPagina = list_remove_by_condition(list_Paginas, (bool (*)(void *)) getSoloInsert)) != NULL){
        stMarco = list_get(listaDeMarcos, auxPagina->nroDePagina);
        stInsert = malloc(sizeof(st_insert));
        stInsert->value = malloc(tamanioValue);
        memcpy(&stInsert->timestamp, auxPagina->pagina, sizeof(double));
        memcpy(&stInsert->key, auxPagina->pagina + sizeof(double), sizeof(uint16_t));
        memcpy(stInsert->value, auxPagina->pagina + sizeof(double) + sizeof(uint16_t), tamanioValue);
        stInsert->operacion = INSERT;
        stInsert->nameTable = strdup(nameTable);

        if(mandarInsert(stInsert) == 5){
        	log_info(file_log, "Value que mando %s", stInsert->value);
            stMarco->condicion = LIBRE;
            free(auxPagina);
        }else {
        	log_info(file_log, "Value que vuelvo a insertar %s", stInsert->value);
            list_add(list_paginas_fallidas, auxPagina);
        }
        destroyInsert(stInsert);
    }
    list_add_all(list_Paginas,list_paginas_fallidas);
    list_destroy(list_paginas_fallidas);
}

int comandoJournal(){
    t_list * list_clone_segmento = list_create();
    st_segmento * auxSegmento = NULL;
    log_info(file_log, "[proceso Journal]Iniciando Journal");
    pthread_mutex_lock(&mutexMemPrinc);
    list_add_all(list_clone_segmento,listaDeSegmentos);
    list_clean(listaDeSegmentos);
    while (list_clone_segmento->elements_count > 0){
        auxSegmento = list_remove(list_clone_segmento,0);
        enviarPaginas(auxSegmento->tablaDePaginas, auxSegmento->nombreTabla);
        if(auxSegmento->tablaDePaginas->elements_count == 0){
            log_info(file_log, "[proceso Journal]Eliminando Segmento name %s",auxSegmento->nombreTabla);
            free(auxSegmento->nombreTabla);
            list_destroy(auxSegmento->tablaDePaginas);
            free(auxSegmento);
        }else{
            log_info(file_log, "[proceso Journal]No se elimina segmento %s",auxSegmento->nombreTabla);
            list_add(listaDeSegmentos,auxSegmento);
        }
    }
    log_info(file_log, "[proceso Journal]Finalizando el Journal");
    pthread_mutex_unlock(&mutexMemPrinc);
    list_destroy(list_clone_segmento);
    return OK;
}

int removerSegmentoPorNombrePagina(char * nameTable){
    log_info(file_log, "[proceso Drop] Iniciando");
    st_segmento * stSegmento = NULL;
    st_tablaDePaginas * stTablaDePaginas;
    pthread_mutex_lock(&mutexMemPrinc);
    bool getSegmento(st_segmento * segmentoAux){
        return strcmp(segmentoAux->nombreTabla, nameTable) == 0;
    }
    log_info(file_log, "[proceso Drop] Buscando segmento");
    stSegmento = list_remove_by_condition(listaDeSegmentos, (bool (*)(void *))getSegmento);
    if(stSegmento){
        log_info(file_log, "[proceso Drop] Eliminando segmento");
        while (stSegmento->tablaDePaginas->elements_count > 0){
            stTablaDePaginas = list_remove(stSegmento->tablaDePaginas, 0);
            st_marco* marco = list_get(listaDeMarcos, stTablaDePaginas->nroDePagina);
            free(stTablaDePaginas);
            marco->condicion = LIBRE;
        }
        list_destroy(stSegmento->tablaDePaginas);
        free(stSegmento->nombreTabla);
        free(stSegmento);

        pthread_mutex_unlock(&mutexMemPrinc);
        sleep(getRetardoMem()/1000);
        log_info(file_log, "[proceso Drop] Finalizando");
        return OK;
    }else{
        log_info(file_log, "[proceso Drop] No se encontro el segmento");
        pthread_mutex_unlock(&mutexMemPrinc);
        log_info(file_log, "[proceso Drop] Finalizando");
        return NOOK;
    }
}

