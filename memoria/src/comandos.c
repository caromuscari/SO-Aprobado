#include "comandos.h"

extern t_log *file_log;
extern int tamanioValue;
extern t_list* listaDeMarcos;
extern void *memoriaPrincipal;
extern t_list* listaDeSegmentos;
extern pthread_mutex_t mutexMemPrinc;
extern t_configuracionMemoria* configMemoria;


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
			memcpy(paginaDeTablaEncontrada->pagina + sizeof(double) + sizeof(uint16_t), comandoInsert->value, tamanioValue);
			memcpy(paginaDeTablaEncontrada->pagina, &comandoInsert->timestamp, sizeof(double));
			paginaDeTablaEncontrada->flagModificado = 1;
            log_info(file_log,"[proceso Insert] Finalizar proceso");
            sleep(configMemoria->RETARDO_MEM/1000);
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
		memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), comandoInsert->value, tamanioValue);

		st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
		paginaDeTabla->nroDePagina = posMarcoLibre;
		paginaDeTabla->pagina = paginaLibre;
		paginaDeTabla->flagModificado = 1;

        log_info(file_log, "[proceso Insert]Agregando nueva pagina al segmento");
        list_add(segmentoEncontrado->tablaDePaginas, paginaDeTabla);
		st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
		marco->condicion = OCUPADO;
        log_info(file_log,"[proceso Insert] Finalizar proceso");
        sleep(configMemoria->RETARDO_MEM/1000);
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
	memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), comandoInsert->value, tamanioValue);
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
    sleep(configMemoria->RETARDO_MEM/1000);
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
			sleep(configMemoria->RETARDO_MEM/1000);
            *resultado = OK;
            log_info(file_log,"[proceso Select] Finalizando Proceso");
            pthread_mutex_unlock(&mutexMemPrinc);
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
		memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), registro->value, tamanioValue);

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
        sleep(configMemoria->RETARDO_MEM/1000);
        *resultado = OK;
        log_info(file_log,"[proceso Select] Finalizando Proceso");
        pthread_mutex_unlock(&mutexMemPrinc);
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
	memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), registro->value, tamanioValue);

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
    sleep(configMemoria->RETARDO_MEM/1000);
    *resultado = OK;
    log_info(file_log,"[proceso Select] Finalizando Proceso");
    pthread_mutex_unlock(&mutexMemPrinc);
	return registro;
}

//COMANDO JOURNAL
bool enviarSegmentoAFS(st_segmento* segmento){
    bool huboError = false;
    void enviarPaginasAFS(st_tablaDePaginas * pagina){
        if(pagina->flagModificado) {
            st_marco *marco = list_get(listaDeMarcos, pagina->nroDePagina);

            st_insert *insert = malloc(sizeof(st_insert));
            insert->value = malloc(tamanioValue);

            memcpy(&insert->timestamp, pagina->pagina, sizeof(double));
            memcpy(&insert->key, pagina->pagina + sizeof(double), sizeof(uint16_t));
        	memcpy(insert->value, pagina->pagina + sizeof(double) + sizeof(uint16_t), tamanioValue);


            insert->operacion = INSERT;
            insert->nameTable = strdup(segmento->nombreTabla);

            if(mandarInsert(insert) == 5){
                //BORRO DE MEMORIA
                marco->condicion = LIBRE;
                free(pagina);
            }else{
             log_error(file_log, string_from_format("El Filesystem rechazó el registro \nTabla:%s | Key:%d | Value:%s \n", insert->nameTable, insert->key, insert->value));
             huboError = true;
            }
            destroyInsert(insert);
        }
    }
    list_iterate(segmento->tablaDePaginas, (void *)enviarPaginasAFS);
    if(!huboError){
        //ELIMINA EL SEGMENTO DE MEMORIA
        list_destroy(segmento->tablaDePaginas);
        free(segmento->nombreTabla);
        free(segmento);
        return true;
    } else {
    	log_info(file_log, "No se pudieron borrar todas las paginas");
    	return false;
    }
}

int comandoJournal(){
	log_info(file_log, "[proceso Journal]Iniciando Journal");
	bool resultado = true;
	st_segmento * segmento;
    pthread_mutex_lock(&mutexMemPrinc);
    for (int i = 0; i < list_size(listaDeSegmentos); i++){
    	segmento = list_get(listaDeSegmentos, i);
    	if(!enviarSegmentoAFS(segmento)){
    		resultado = false;
    	} else {
    		list_remove(listaDeSegmentos, i);
    	}
    }
	//pthread_mutex_unlock(&mutexListaSeg);
    if(resultado){
    log_info(file_log, "[proceso Journal]Finalizando el Journal");
    pthread_mutex_unlock(&mutexMemPrinc);
    return OK;
    }
    log_error(file_log, "No se pudo realizar el Journal");
    pthread_mutex_unlock(&mutexMemPrinc);
    return NOOK;
}

int removerSegmentoPorNombrePagina(char* nombreTabla){
	pthread_mutex_lock(&mutexMemPrinc);
	  st_segmento* segmentoEncontrado = buscarSegmentoPorNombreTabla(nombreTabla);
	    if(segmentoEncontrado){
	        log_info(file_log, "Se encontro el segmento por Drop");

	        for(int i = 0; i < list_size(segmentoEncontrado->tablaDePaginas); i++){
	            st_tablaDePaginas* paginaDeTabla = list_get(segmentoEncontrado->tablaDePaginas, i);
	            st_marco* marco = list_get(listaDeMarcos, paginaDeTabla->nroDePagina);
	            free(paginaDeTabla);
	            marco->condicion = LIBRE;
	        }

	        list_destroy(segmentoEncontrado->tablaDePaginas);
	        free(segmentoEncontrado->nombreTabla);
	        st_segmento *seg = list_remove(listaDeSegmentos, segmentoEncontrado->nroSegmento);
	        free(seg->nombreTabla);
	        list_clean_and_destroy_elements(seg->tablaDePaginas, free);
	        list_destroy(seg->tablaDePaginas);
	        free(seg);
	        for(int i = segmentoEncontrado->nroSegmento; i < list_size(listaDeSegmentos); i++){
	            st_segmento* segmento = list_get(listaDeSegmentos, i);
	            int nro = segmento->nroSegmento;
	            segmento->nroSegmento = nro - 1;
	        }
	        sleep(configMemoria->RETARDO_MEM/1000);
	        pthread_mutex_unlock(&mutexMemPrinc);
	        return OK;
	    } else {
	    	log_info(file_log, "No se encontro el segmento por Drop");
	    	pthread_mutex_unlock(&mutexMemPrinc);
	    	return NOOK;
	    }
}

