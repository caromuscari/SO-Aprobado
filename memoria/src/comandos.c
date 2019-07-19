#include "comandos.h"

extern t_log *file_log;
extern int tamanioValue;
extern t_list* listaDeMarcos;
extern void *memoriaPrincipal;
extern t_list* listaDeSegmentos;


// INICIALIZO MEMORIA

void inicializarMemoria(){
	listaDeSegmentos = list_create();
	log_info(file_log, "La lista de segmentos se creo correctamente\n");
}

//COMANDO INSERT

int comandoInsert(st_insert* comandoInsert){
	if(strlen(comandoInsert->value) > tamanioValue){
		log_error(file_log, "El value es mayor al tamaño máximo\n");
		return MAYORQUEVALUEMAX;
	}
	st_segmento* segmentoEncontrado = buscarSegmentoPorNombreTabla(comandoInsert->nameTable);//devuelve el segmento con ese nombre de tabla
	if(segmentoEncontrado){
		log_info(file_log, "Segmento encontrado por comando Insert\n");
		st_tablaDePaginas* paginaDeTablaEncontrada = buscarPaginaPorKey(segmentoEncontrado->tablaDePaginas, comandoInsert->key);
		if(paginaDeTablaEncontrada){
			log_info(file_log, "Pagina encontrada\n");
			memcpy(paginaDeTablaEncontrada->pagina + sizeof(double) + sizeof(uint16_t), comandoInsert->value, string_length(comandoInsert->value));
			memcpy(paginaDeTablaEncontrada->pagina, &comandoInsert->timestamp, sizeof(double));
			paginaDeTablaEncontrada->flagModificado = 1;

			log_info(file_log, "El Insert se realizo correctamente\n");
			return OK;
		}
		log_info(file_log, "No se encontro la pagina con esa Key\n");

		int posMarcoLibre = buscarMarcoLibre();
		if(posMarcoLibre == -1){
            return FULLMEMORY;
		}

		void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));

		memcpy(paginaLibre, &comandoInsert->timestamp, sizeof(double));
		memcpy(paginaLibre + sizeof(double), &comandoInsert->key, sizeof(uint16_t));
		memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), comandoInsert->value, string_length(comandoInsert->value));

		st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
		paginaDeTabla->nroDePagina = posMarcoLibre;
		paginaDeTabla->pagina = paginaLibre;
		paginaDeTabla->desplazamiento = sizeof(double) + sizeof(uint16_t) + string_length(comandoInsert->value);
		paginaDeTabla->flagModificado = 1;

		list_add(segmentoEncontrado->tablaDePaginas, paginaDeTabla);

		st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
		marco->condicion = OCUPADO;
        mostrarPaginasCargadas();
        log_info(file_log, "El Insert se realizo correctamente\n");
		return OK;
	}
	log_info(file_log, "No se encontro el segmento de esa tabla\n");
	//creo el segmento
	st_segmento* segmentoNuevo= malloc(sizeof(st_segmento));

	segmentoNuevo->nombreTabla = strdup(comandoInsert->nameTable);
	segmentoNuevo->tablaDePaginas = list_create();

	int posMarcoLibre = buscarMarcoLibre();
	if(posMarcoLibre == -1){
        return FULLMEMORY;
	}
	//creo la pagina
	void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));
	//cargo datos a la memoria princ
	memcpy(paginaLibre, &comandoInsert->timestamp, sizeof(double));
	memcpy(paginaLibre + sizeof(double), &comandoInsert->key, sizeof(uint16_t));
	memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), comandoInsert->value, string_length(comandoInsert->value));
	//creo la pag de la tabla y le cargo los datos
	st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
	paginaDeTabla->nroDePagina = posMarcoLibre;
	paginaDeTabla->pagina = paginaLibre;
    paginaDeTabla->desplazamiento = sizeof(double) + sizeof(uint16_t) + string_length(comandoInsert->value);
	paginaDeTabla->flagModificado = 1;
	//agrego la pag a la lista
	list_add(segmentoNuevo->tablaDePaginas, paginaDeTabla);
	//agrego el segmento a la lista de segmentos en el ultimo lugar
	segmentoNuevo->nroSegmento = list_size(listaDeSegmentos);
	list_add_in_index(listaDeSegmentos, segmentoNuevo->nroSegmento, segmentoNuevo);

	st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
	marco->condicion = OCUPADO;
	log_info(file_log, "El Insert se realizo correctamente\n");
	return OK;
}


// COMANDO SELECT

st_registro* comandoSelect(st_select* comandoSelect){
	st_segmento* segmentoEncontrado;
	st_registro* registro;
	segmentoEncontrado = buscarSegmentoPorNombreTabla(comandoSelect->nameTable);//devuelve el segmento con ese nombre de tabla
	if(segmentoEncontrado){
		log_info(file_log, "Segmento encontrado por comando Select\n");
		st_tablaDePaginas* paginaDeTablaEncontrada = buscarPaginaPorKey(segmentoEncontrado->tablaDePaginas, comandoSelect->key);
		if(paginaDeTablaEncontrada){
			log_info(file_log, "Pagina encontrada por comando Select\n");
			int sizeValue = paginaDeTablaEncontrada->desplazamiento - (sizeof(double) + sizeof(uint16_t));
			registro = malloc(sizeof(st_registro));
			registro->value = malloc(sizeValue);
			st_marco* marco = list_get(listaDeMarcos, paginaDeTablaEncontrada->nroDePagina);
			marco->timestamp = obtenerMilisegundosDeHoy();

			memcpy(&registro->timestamp, paginaDeTablaEncontrada->pagina, sizeof(double));
			memcpy(&registro->key, paginaDeTablaEncontrada->pagina+sizeof(double), sizeof(uint16_t));
			memcpy(registro->value, paginaDeTablaEncontrada->pagina+sizeof(double)+sizeof(uint16_t), sizeValue);

			return registro;
		}
		log_info(file_log, "No se encontro la pagina con esa Key\n");

		registro = obtenerSelect(comandoSelect);

		if(registro == NULL){
			return NULL;
		}

		int posMarcoLibre = buscarMarcoLibre();

		void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));

		memcpy(paginaLibre, &registro->timestamp, sizeof(double));
		memcpy(paginaLibre + sizeof(double), &registro->key, sizeof(uint16_t));
		memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), registro->value, string_length(registro->value));

		st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
		paginaDeTabla->nroDePagina = posMarcoLibre;
		paginaDeTabla->pagina = paginaLibre;
		paginaDeTabla->desplazamiento = sizeof(double) + sizeof(uint16_t) + string_length(registro->value);
		paginaDeTabla->flagModificado = 0;

		list_add(segmentoEncontrado->tablaDePaginas, paginaDeTabla);

		st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
		marco->condicion = OCUPADO;
		marco->timestamp = obtenerMilisegundosDeHoy();
        mostrarPaginasCargadas();
		return registro;
	}
	log_info(file_log, "No se encontro el segmento de la tabla pedida por Select\n");
	registro = obtenerSelect(comandoSelect);

	if(registro == NULL){
		return NULL;
	}

	st_segmento* segmentoNuevo= malloc(sizeof(st_segmento));

	segmentoNuevo->nombreTabla = strdup(comandoSelect->nameTable);
	segmentoNuevo->tablaDePaginas = list_create();

	int posMarcoLibre = buscarMarcoLibre();
	//creo la pagina
	void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));

	memcpy(paginaLibre, &registro->timestamp, sizeof(double));
	memcpy(paginaLibre + sizeof(double), &registro->key, sizeof(uint16_t));
	memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), registro->value, string_length(registro->value));

	st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
	paginaDeTabla->nroDePagina = posMarcoLibre;
	paginaDeTabla->pagina = paginaLibre;
    paginaDeTabla->desplazamiento = sizeof(double) + sizeof(uint16_t) + string_length(registro->value);
	paginaDeTabla->flagModificado = 0;

	list_add(segmentoNuevo->tablaDePaginas, paginaDeTabla);

	//agrego el segmento a la lista de segmentos en el ultimo lugar
	segmentoNuevo->nroSegmento = list_size(listaDeSegmentos);
	list_add_in_index(listaDeSegmentos, segmentoNuevo->nroSegmento, segmentoNuevo);

	st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
	marco->condicion = OCUPADO;
	marco->timestamp = obtenerMilisegundosDeHoy();
	mostrarPaginasCargadas();

	return registro;
}

/*
int comandoDrop(st_drop* comandoDrop){
	st_segmento* segmentoEncontrado = buscarSegmentoPorNombreTabla(comandoDrop->nameTable);
	if(segmentoEncontrado){
		log_info(file_log, "Se encontro el segmento por Drop");

		for(int i = 0; i < list_size(segmentoEncontrado->tablaDePaginas); i++){
			st_tablaDePaginas* paginaDeTabla = list_get(segmentoEncontrado->tablaDePaginas, i);
			free(paginaDeTabla->pagina);
			st_marco* marco = list_get(listaDeMarcos, paginaDeTabla->nroDePagina);
			marco->condicion = LIBRE;
		}

		list_destroy(segmentoEncontrado->tablaDePaginas);
		free(segmentoEncontrado->tablaDePaginas);
		free(segmentoEncontrado->nombreTabla);
		list_remove(listaDeSegmentos, segmentoEncontrado->nroSegmento);
		for(int i = segmentoEncontrado->nroSegmento; i < list_size(listaDeSegmentos); i++){
			st_segmento* segmento = list_get(listaDeSegmentos, i);
			int nro = segmento->nroSegmento;
			segmento->nroSegmento = nro - 1;
		}
		return 0;
	} else {
		//se informa al file system
		informarDrop(comandoDrop);
		return 0;
	}
}*/

//COMANDO JOURNAL
bool enviarSegmentoAFS(st_segmento* segmento){
    bool huboError = false;
    void enviarPaginasAFS(st_tablaDePaginas * pagina){
        if(pagina->flagModificado) {
            st_marco *marco = list_get(listaDeMarcos, pagina->nroDePagina);
            st_insert *insert = malloc(sizeof(st_insert));
            int sizeValue = pagina->desplazamiento - (sizeof(double) + sizeof(uint16_t));
            insert->value = malloc(sizeValue);

            memcpy(&insert->timestamp, pagina->pagina, sizeof(double));
            memcpy(&insert->key, pagina->pagina + sizeof(double), sizeof(uint16_t));
            memcpy(insert->value, pagina->pagina + sizeof(double) + sizeof(uint16_t), sizeValue);
            insert->operacion = INSERT;
            insert->nameTable = strdup(segmento->nombreTabla);

            if(mandarInsert(insert) == 5){
                //BORRO DE MEMORIA
                marco->condicion = LIBRE;
                free(pagina);
            }else{
             log_error(file_log, string_from_format("El Filesystem rechazó el registro \nTabla:%s | Key:%d | Value:%s\n", insert->nameTable, insert->key, insert->value));
             huboError = true;
            }
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
    	log_info(file_log, "No se pudieron borrar todas las paginas\n");
    	return false;
    }
}

int comandoJournal(){
    //RECORRO TODA LA MEMORIA Y HAGO LOS INSERT CORRESPONDIENTES COMO REQUEST AL FILE
	log_info(file_log, "Ejecutando Journal\n");
	bool resultado = true;
	st_segmento * segmento;
	for (int i = 0; i < list_size(listaDeSegmentos); i++){
    	segmento = list_get(listaDeSegmentos, i);
    	if(!enviarSegmentoAFS(segmento)){
    		resultado = false;
    	} else {
    		list_remove(listaDeSegmentos, i);
    	}
    }
    if(resultado){
    log_info(file_log, "Termino el Journal\n");
    return OK;
    }
    log_error(file_log, "No se pudo realizar el Journal");
    return NOOK;
}

