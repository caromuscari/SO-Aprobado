#include "comandos.h"

extern t_log *file_log;
extern int tamanioValue;
extern t_list* listaDeMarcos;
extern void *memoriaPrincipal;
extern t_list* listaDeSegmentos;


// INICIALIZO MEMORIA

void inicializarMemoria(){
	listaDeSegmentos = list_create();
	log_info(file_log, "se creo la lista de segmentos");
}

//COMANDO INSERT

int comandoInsert(st_insert* comandoInsert){
	if(strlen(comandoInsert->value) > tamanioValue){
		log_info(file_log, "El value es mayor al tamaño máximo.");
		return -1;
	}
	st_segmento* segmentoEncontrado = buscarSegmentoPorNombreTabla(comandoInsert->nameTable);//devuelve el segmento con ese nombre de tabla
	if(segmentoEncontrado){
		log_info(file_log, "Segmento encontrado por comando Insert");
		st_tablaDePaginas* paginaDeTablaEncontrada = buscarPaginaPorKey(segmentoEncontrado->tablaDePaginas, comandoInsert->key);
		if(paginaDeTablaEncontrada){
			log_info(file_log, "Pagina encontrada");
			memcpy(paginaDeTablaEncontrada->pagina + sizeof(double) + sizeof(uint16_t), comandoInsert->value, tamanioValue);
			memcpy(paginaDeTablaEncontrada->pagina, &comandoInsert->timestamp, sizeof(double));
			paginaDeTablaEncontrada->flagModificado = 1;

			return 0;
		}
		log_info(file_log, "No se encontro la pagina con esa Key");

		int posMarcoLibre = buscarMarcoLibre();
		if(posMarcoLibre == -1){
            return -2;
		}

		void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));

		memcpy(paginaLibre, &comandoInsert->timestamp, sizeof(double));
		memcpy(paginaLibre + sizeof(double), &comandoInsert->key, sizeof(uint16_t));
		memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), comandoInsert->value, tamanioValue);

		st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
		paginaDeTabla->nroDePagina = posMarcoLibre;
		paginaDeTabla->pagina = paginaLibre;
		paginaDeTabla->flagModificado = 1;

		list_add(segmentoEncontrado->tablaDePaginas, paginaDeTabla);

		st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
		marco->condicion = OCUPADO;
        mostrarPaginasCargadas();
		return 0;
	}
	log_info(file_log, "No se encontro el segmento de esa tabla");
	//creo el segmento
	st_segmento* segmentoNuevo= malloc(sizeof(st_segmento));

	segmentoNuevo->nombreTabla = strdup(comandoInsert->nameTable);
	segmentoNuevo->tablaDePaginas = list_create();

	int posMarcoLibre = buscarMarcoLibre();
	if(posMarcoLibre == -1){
        return -2;
	}
	//creo la pagina
	void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));
	//cargo datos a la memoria princ
	memcpy(paginaLibre, &comandoInsert->timestamp, sizeof(double));
	memcpy(paginaLibre + sizeof(double), &comandoInsert->key, sizeof(uint16_t));
	memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), comandoInsert->value, tamanioValue);
	//creo la pag de la tabla y le cargo los datos
	st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
	paginaDeTabla->nroDePagina = posMarcoLibre;
	paginaDeTabla->pagina = paginaLibre;
	paginaDeTabla->flagModificado = 1;
	//agrego la pag a la lista
	list_add(segmentoNuevo->tablaDePaginas, paginaDeTabla);
	//agrego el segmento a la lista de segmentos en el ultimo lugar
	segmentoNuevo->nroSegmento = list_size(listaDeSegmentos);
	list_add_in_index(listaDeSegmentos, segmentoNuevo->nroSegmento, segmentoNuevo);

	st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
	marco->condicion = OCUPADO;
    mostrarPaginasCargadas();
	return 0;
}


// COMANDO SELECT

st_registro* comandoSelect(st_select* comandoSelect){
	st_registro* registro = malloc(sizeof(st_registro));
	registro->value = malloc(tamanioValue);
	st_segmento* segmentoEncontrado;
	segmentoEncontrado = buscarSegmentoPorNombreTabla(comandoSelect->nameTable);//devuelve el segmento con ese nombre de tabla
	if(segmentoEncontrado){
		log_info(file_log, "Segmento encontrado por comando Select");
		st_tablaDePaginas* paginaDeTablaEncontrada = buscarPaginaPorKey(segmentoEncontrado->tablaDePaginas, comandoSelect->key);
		if(paginaDeTablaEncontrada){
			log_info(file_log, "Pagina encontrada por comando Select");

			st_marco* marco = list_get(listaDeMarcos, paginaDeTablaEncontrada->nroDePagina);
			marco->timestamp = obtenerMilisegundosDeHoy();

			memcpy(&registro->timestamp, paginaDeTablaEncontrada->pagina, sizeof(double));
			memcpy(&registro->key, paginaDeTablaEncontrada->pagina+sizeof(double), sizeof(uint16_t));
			memcpy(registro->value, paginaDeTablaEncontrada->pagina+sizeof(double)+sizeof(uint16_t), tamanioValue);

			return registro;
		}
		log_info(file_log, "No se encontro la pagina con esa Key");

		registro = obtenerSelect(comandoSelect);

		if(registro == NULL){
			return NULL;
		}

		int posMarcoLibre = buscarMarcoLibre();

		void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));

		memcpy(paginaLibre, &registro->timestamp, sizeof(double));
		memcpy(paginaLibre + sizeof(double), &registro->key, sizeof(uint16_t));
		memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), registro->value, tamanioValue);

		st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
		paginaDeTabla->nroDePagina = posMarcoLibre;
		paginaDeTabla->pagina = paginaLibre;
		paginaDeTabla->flagModificado = 0;

		list_add(segmentoEncontrado->tablaDePaginas, paginaDeTabla);

		st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
		marco->condicion = OCUPADO;
		marco->timestamp = obtenerMilisegundosDeHoy();
        mostrarPaginasCargadas();
		return registro;
	}
	log_info(file_log, "No se encontro el segmento de esa tabla");
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
	memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), registro->value, tamanioValue);

	st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
	paginaDeTabla->nroDePagina = posMarcoLibre;
	paginaDeTabla->pagina = paginaLibre;
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
void* enviarSegmentoAFS(st_segmento* segmento){
    bool huboError = false;
    void* enviarPaginasAFS(st_tablaDePaginas * pagina){
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
             log_error(file_log, string_from_format("El Filesystem rechazó el registro \nTabla:%s | Key:%d | Value:%s", insert->nameTable, insert->key, insert->value));
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
    }
}

int comandoJournal(){
    //RECORRO TODA LA MEMORIA Y HAGO LOS INSERT CORRESPONDIENTES COMO REQUEST AL FILE
    list_iterate(listaDeSegmentos, (void*)enviarSegmentoAFS);
    log_info(file_log, "Terminó el JOURNAL");
    list_clean(listaDeSegmentos);
    return 1;
}

