#include "comandos.h"

extern t_log *file_log;
extern int tamanioValue;
extern t_list* listaDeMarcos;
extern void *memoriaPrincipal;
extern t_list* listaDeSegmentos;
extern pthread_mutex_t mutexListaSeg, mutexListaMarcos, mutexMemPrinc;
extern t_configuracionMemoria* configMemoria;


//COMANDO INSERT
int comandoInsert(st_insert* comandoInsert){
	if(strlen(comandoInsert->value) > tamanioValue){
		log_error(file_log, "El value es mayor al tamaño máximo");
		return MAYORQUEVALUEMAX;
	}
    pthread_mutex_lock(&mutexListaSeg);

	st_segmento* segmentoEncontrado = buscarSegmentoPorNombreTabla(comandoInsert->nameTable);//devuelve el segmento con ese nombre de tabla
	if(segmentoEncontrado){
		log_info(file_log, "Segmento encontrado por comando Insert");

		st_tablaDePaginas* paginaDeTablaEncontrada = buscarPaginaPorKey(segmentoEncontrado->tablaDePaginas, comandoInsert->key);

		if(paginaDeTablaEncontrada){
			log_info(file_log, "Pagina encontrada");
			pthread_mutex_lock(&mutexMemPrinc);
			memcpy(paginaDeTablaEncontrada->pagina + sizeof(double) + sizeof(uint16_t), comandoInsert->value, tamanioValue);
			memcpy(paginaDeTablaEncontrada->pagina, &comandoInsert->timestamp, sizeof(double));
			paginaDeTablaEncontrada->flagModificado = 1;
			//paginaDeTablaEncontrada->desplazamiento = string_length(comandoInsert->value);

			pthread_mutex_unlock(&mutexMemPrinc);
			log_info(file_log, "El Insert se realizo correctamente");

            sleep(configMemoria->RETARDO_MEM/1000);
            pthread_mutex_unlock(&mutexListaSeg);
			return OK;
		}
		log_info(file_log, "No se encontro la pagina con esa Key");
		int posMarcoLibre = buscarMarcoLibre();
		if(posMarcoLibre == -1){
			log_info(file_log, "Memoria full por insert");
			pthread_mutex_unlock(&mutexListaSeg);
            return FULLMEMORY;
		}
		pthread_mutex_lock(&mutexMemPrinc);
		void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));

		memcpy(paginaLibre, &comandoInsert->timestamp, sizeof(double));
		memcpy(paginaLibre + sizeof(double), &comandoInsert->key, sizeof(uint16_t));

		int largo_value = strlen(comandoInsert->value);
		comandoInsert->value[largo_value]='\0';

		memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), comandoInsert->value, tamanioValue);

		st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
		paginaDeTabla->nroDePagina = posMarcoLibre;
		paginaDeTabla->pagina = paginaLibre;
		//paginaDeTabla->desplazamiento =string_length(comandoInsert->value);
		paginaDeTabla->flagModificado = 1;

		list_add(segmentoEncontrado->tablaDePaginas, paginaDeTabla);
		pthread_mutex_unlock(&mutexListaSeg);
		pthread_mutex_unlock(&mutexMemPrinc);
		pthread_mutex_lock(&mutexListaMarcos);
		st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
		marco->condicion = OCUPADO;
		pthread_mutex_unlock(&mutexListaMarcos);
        log_info(file_log, "El Insert se realizo correctamente");
        sleep(configMemoria->RETARDO_MEM/1000);
		return OK;
	}

	log_info(file_log, "No se encontro el segmento de esa tabla");
	//creo el segmento
	st_segmento* segmentoNuevo= malloc(sizeof(st_segmento));

	segmentoNuevo->nombreTabla = strdup(comandoInsert->nameTable);
	segmentoNuevo->tablaDePaginas = list_create();

	int posMarcoLibre = buscarMarcoLibre();
	if(posMarcoLibre == -1){
		log_info(file_log, "Memoria full insert");
		pthread_mutex_unlock(&mutexListaSeg);
        return FULLMEMORY;
	}
	pthread_mutex_unlock(&mutexListaSeg);
	//creo la pagina
	pthread_mutex_lock(&mutexMemPrinc);
	void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));
	//cargo datos a la memoria princ
	memcpy(paginaLibre, &comandoInsert->timestamp, sizeof(double));
	memcpy(paginaLibre + sizeof(double), &comandoInsert->key, sizeof(uint16_t));

	int largo_value = strlen(comandoInsert->value);
	comandoInsert->value[largo_value]='\0';

	memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), comandoInsert->value, tamanioValue);
	//creo la pag de la tabla y le cargo los datos
	st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
	paginaDeTabla->nroDePagina = posMarcoLibre;
	paginaDeTabla->pagina = paginaLibre;
    //paginaDeTabla->desplazamiento =string_length(comandoInsert->value);
	paginaDeTabla->flagModificado = 1;
	//agrego la pag a la lista
	pthread_mutex_lock(&mutexListaSeg);
	list_add(segmentoNuevo->tablaDePaginas, paginaDeTabla);
	pthread_mutex_unlock(&mutexMemPrinc);
	//agrego el segmento a la lista de segmentos en el ultimo lugar
	segmentoNuevo->nroSegmento = list_size(listaDeSegmentos);
	list_add_in_index(listaDeSegmentos, segmentoNuevo->nroSegmento, segmentoNuevo);
	pthread_mutex_unlock(&mutexListaSeg);
	pthread_mutex_lock(&mutexListaMarcos);
	st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
	marco->condicion = OCUPADO;
	pthread_mutex_unlock(&mutexListaMarcos);
	log_info(file_log, "El Insert se realizo correctamente");
    sleep(configMemoria->RETARDO_MEM/1000);
	return OK;
}

// COMANDO SELECT
st_registro* comandoSelect(st_select* comandoSelect, enum_resultados* resultado){
	st_segmento* segmentoEncontrado;
	st_registro* registro;
    pthread_mutex_lock(&mutexListaSeg);
	segmentoEncontrado = buscarSegmentoPorNombreTabla(comandoSelect->nameTable);//devuelve el segmento con ese nombre de tabla
	pthread_mutex_unlock(&mutexListaSeg);
	if(segmentoEncontrado){
		log_info(file_log, "Segmento encontrado por comando Select");
		st_tablaDePaginas* paginaDeTablaEncontrada = buscarPaginaPorKey(segmentoEncontrado->tablaDePaginas, comandoSelect->key);
		if(paginaDeTablaEncontrada){
			log_info(file_log, "Pagina encontrada por comando Select");
			registro = malloc(sizeof(st_registro));
			pthread_mutex_lock(&mutexMemPrinc);
            registro->value = malloc(tamanioValue);
			st_marco* marco = list_get(listaDeMarcos, paginaDeTablaEncontrada->nroDePagina);
			marco->timestamp = obtenerMilisegundosDeHoy();
			memcpy(&registro->timestamp, paginaDeTablaEncontrada->pagina, sizeof(double));
			memcpy(&registro->key, paginaDeTablaEncontrada->pagina+sizeof(double), sizeof(uint16_t));
			memcpy(registro->value, paginaDeTablaEncontrada->pagina+sizeof(double)+sizeof(uint16_t), tamanioValue);

			printf("El value es [%s] (tengo segmento y pag) \n", registro->value);
            pthread_mutex_unlock(&mutexMemPrinc);
            sleep(configMemoria->RETARDO_MEM/1000);
            *resultado = OK;
			return registro;
		}
		log_info(file_log, "No se encontro la pagina con esa Key");

		registro = obtenerSelect(comandoSelect);

		if(registro == NULL){
			*resultado = NOOK;
			return NULL;
		}
        pthread_mutex_lock(&mutexListaSeg);
		int posMarcoLibre = buscarMarcoLibre();
        pthread_mutex_unlock(&mutexListaSeg);

    	if(posMarcoLibre == -1){
    		log_info(file_log, "Memoria full por Select");
    		*resultado = FULLMEMORY;
            return NULL;
    	}

		void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));

		pthread_mutex_lock(&mutexMemPrinc);
		memcpy(paginaLibre, &registro->timestamp, sizeof(double));
		memcpy(paginaLibre + sizeof(double), &registro->key, sizeof(uint16_t));

		int largo_value = strlen(registro->value);
		registro->value[largo_value]='\0';
		memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), registro->value, tamanioValue);
		pthread_mutex_unlock(&mutexMemPrinc);

		st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
		paginaDeTabla->nroDePagina = posMarcoLibre;
		paginaDeTabla->pagina = paginaLibre;
		//paginaDeTabla->desplazamiento = string_length(registro->value);
		paginaDeTabla->flagModificado = 0;

		list_add(segmentoEncontrado->tablaDePaginas, paginaDeTabla);

		pthread_mutex_lock(&mutexListaMarcos);
		st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
		marco->condicion = OCUPADO;
		marco->timestamp = obtenerMilisegundosDeHoy();
		pthread_mutex_unlock(&mutexListaMarcos);
        sleep(configMemoria->RETARDO_MEM/1000);

        printf("El value es [%s] (tengo el seg, pido a file)\n", registro->value);

        *resultado = OK;
		return registro;
	}
	log_info(file_log, "No se encontro el segmento de la tabla pedida por Select");
	registro = obtenerSelect(comandoSelect);

	if(registro == NULL){
		*resultado = NOOK;
		return NULL;
	}

	st_segmento* segmentoNuevo= malloc(sizeof(st_segmento));

	segmentoNuevo->nombreTabla = strdup(comandoSelect->nameTable);
	segmentoNuevo->tablaDePaginas = list_create();
    pthread_mutex_lock(&mutexListaSeg);
	int posMarcoLibre = buscarMarcoLibre();
	pthread_mutex_unlock(&mutexListaSeg);

	if(posMarcoLibre == -1){
		log_info(file_log, "Memoria full por Select");
		pthread_mutex_unlock(&mutexListaSeg);
		*resultado = FULLMEMORY;
        return NULL;
	}

	//creo la pagina
	void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));
    pthread_mutex_lock(&mutexMemPrinc);
	memcpy(paginaLibre, &registro->timestamp, sizeof(double));
	memcpy(paginaLibre + sizeof(double), &registro->key, sizeof(uint16_t));

	int largo_value = strlen(registro->value);
	registro->value[largo_value]='\0';
	memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), registro->value, tamanioValue);
	pthread_mutex_unlock(&mutexMemPrinc);

	st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
	paginaDeTabla->nroDePagina = posMarcoLibre;
	paginaDeTabla->pagina = paginaLibre;
    //paginaDeTabla->desplazamiento = string_length(registro->value);
	paginaDeTabla->flagModificado = 0;

	pthread_mutex_lock(&mutexListaSeg);
	list_add(segmentoNuevo->tablaDePaginas, paginaDeTabla);
	//agrego el segmento a la lista de segmentos en el ultimo lugar
	segmentoNuevo->nroSegmento = list_size(listaDeSegmentos);
	list_add_in_index(listaDeSegmentos, segmentoNuevo->nroSegmento, segmentoNuevo);
    pthread_mutex_unlock(&mutexListaSeg);

    pthread_mutex_lock(&mutexListaMarcos);
	st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
	marco->condicion = OCUPADO;
	marco->timestamp = obtenerMilisegundosDeHoy();
    pthread_mutex_unlock(&mutexListaMarcos);
    sleep(configMemoria->RETARDO_MEM/1000);

    printf("El value es [%s] (no tengo nada, pido a file)\n", registro->value);

    *resultado = OK;
	return registro;
}

//COMANDO JOURNAL
bool enviarSegmentoAFS(st_segmento* segmento){
    bool huboError = false;
    void enviarPaginasAFS(st_tablaDePaginas * pagina){
        if(pagina->flagModificado) {
            pthread_mutex_lock(&mutexListaMarcos);
            st_marco *marco = list_get(listaDeMarcos, pagina->nroDePagina);

            st_insert *insert = malloc(sizeof(st_insert));
            insert->value = malloc(tamanioValue);

            pthread_mutex_lock(&mutexMemPrinc);
            memcpy(&insert->timestamp, pagina->pagina, sizeof(double));
            memcpy(&insert->key, pagina->pagina + sizeof(double), sizeof(uint16_t));
        	memcpy(insert->value, pagina->pagina + sizeof(double) + sizeof(uint16_t), tamanioValue);

            pthread_mutex_unlock(&mutexMemPrinc);

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
            pthread_mutex_unlock(&mutexListaMarcos);
            destroyInsert(insert);
        }
    }
    list_iterate(segmento->tablaDePaginas, (void *)enviarPaginasAFS);
    if(!huboError){
        //ELIMINA EL SEGMENTO DE MEMORIA
        pthread_mutex_lock(&mutexListaSeg);
        list_destroy(segmento->tablaDePaginas);
        free(segmento->nombreTabla);
        free(segmento);
        pthread_mutex_unlock(&mutexListaSeg);
        return true;
    } else {
    	log_info(file_log, "No se pudieron borrar todas las paginas");
    	return false;
    }
}

int comandoJournal(){
    //RECORRO TODA LA MEMORIA Y HAGO LOS INSERT CORRESPONDIENTES COMO REQUEST AL FILE
	log_info(file_log, "Ejecutando Journal");
	bool resultado = true;
	st_segmento * segmento;

	//pthread_mutex_lock(&mutexListaSeg);
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
    log_info(file_log, "Termino el Journal");
    return OK;
    }
    log_error(file_log, "No se pudo realizar el Journal");
    return NOOK;
}


int removerSegmentoPorNombrePagina(char* nombreTabla){
	  pthread_mutex_lock(&mutexListaSeg);
	  st_segmento* segmentoEncontrado = buscarSegmentoPorNombreTabla(nombreTabla);
	    if(segmentoEncontrado){
	        log_info(file_log, "Se encontro el segmento por Drop");

	        pthread_mutex_lock(&mutexMemPrinc);
	        for(int i = 0; i < list_size(segmentoEncontrado->tablaDePaginas); i++){
	            st_tablaDePaginas* paginaDeTabla = list_get(segmentoEncontrado->tablaDePaginas, i);
	            pthread_mutex_lock(&mutexListaMarcos);
	            st_marco* marco = list_get(listaDeMarcos, paginaDeTabla->nroDePagina);
	            free(paginaDeTabla);
	            marco->condicion = LIBRE;
	            pthread_mutex_unlock(&mutexListaMarcos);
	        }
	        pthread_mutex_unlock(&mutexListaSeg);
	        pthread_mutex_unlock(&mutexMemPrinc);

	        list_destroy(segmentoEncontrado->tablaDePaginas);
	        free(segmentoEncontrado->nombreTabla);
	        pthread_mutex_lock(&mutexListaSeg);
	        st_segmento *seg = list_remove(listaDeSegmentos, segmentoEncontrado->nroSegmento);
	        pthread_mutex_unlock(&mutexListaSeg);
	        free(seg->nombreTabla);
	        list_clean_and_destroy_elements(seg->tablaDePaginas, free);
	        list_destroy(seg->tablaDePaginas);
	        free(seg);
	        pthread_mutex_lock(&mutexListaSeg);
	        for(int i = segmentoEncontrado->nroSegmento; i < list_size(listaDeSegmentos); i++){
	            st_segmento* segmento = list_get(listaDeSegmentos, i);
	            int nro = segmento->nroSegmento;
	            segmento->nroSegmento = nro - 1;
	        }
	        pthread_mutex_unlock(&mutexListaSeg);
	        sleep(configMemoria->RETARDO_MEM/1000);
	        return OK;
	    } else {
	    	log_info(file_log, "No se encontro el segmento por Drop");
	    	pthread_mutex_unlock(&mutexListaSeg);
	    	return NOOK;
	    }
}

