#include "segmentacionPaginada.h"

extern t_list* listaDeMarcos;
extern t_list* listaDeSegmentos;
extern t_log *file_log;

st_segmento* buscarSegmentoPorNombreTabla(char* nombreTabla){
	bool mismoNombreTabla(st_segmento* segmento){
		return 0 == strcmp(segmento->nombreTabla, nombreTabla);
	}
	return (st_segmento*)list_find(listaDeSegmentos, (void*)mismoNombreTabla);
}

st_tablaDePaginas* buscarPaginaPorKey(t_list* tablaDePaginas, uint16_t key){
	bool paginaConEsaKey(st_tablaDePaginas * tablaPagina){
		uint16_t keyAComparar;
		memcpy(&keyAComparar, tablaPagina->pagina + sizeof(double), sizeof(uint16_t));
		return key == keyAComparar;
	}
	return (st_tablaDePaginas*) list_find(tablaDePaginas, (void *)paginaConEsaKey);
}

int buscarMarcoLibre(){
	st_marco* marco;
	log_info(file_log, "Buscando marco libre");
	for(int i = 0; i < listaDeMarcos->elements_count; i++){
		marco = list_get(listaDeMarcos, i);
		if(marco->condicion == LIBRE){
			log_info(file_log, "Retornando marco libre %d", i);
			return i;
		}
	}
	log_info(file_log, "No se encontro marco libre");
	int posMarcoLibre = algoritmoLRU(); //encuentro el marco de la pagina que puedo reemplazar porque se uso hace mas tiempo
	return posMarcoLibre;
}

int algoritmoLRU(){
	log_info(file_log, "Iniciando LRU, size listaSegmento %d", listaDeSegmentos->elements_count);
	t_list* paginasConFlagEnCero = list_create();
	int nro;
	for(int i = 0; i < listaDeSegmentos->elements_count; i++){
		st_segmento* segmento = list_get(listaDeSegmentos, i);
		t_list * pagsAUnir = list_filter(segmento->tablaDePaginas, (void*)tieneFlagEnCero);
		list_add_all(paginasConFlagEnCero, pagsAUnir);
		list_destroy(pagsAUnir);
	}
	if(!list_is_empty(paginasConFlagEnCero)){
		st_tablaDePaginas* paginaAReemplazar = list_get(paginasConFlagEnCero, 0);
		for(int j = 0; j < paginasConFlagEnCero->elements_count; j++){
			paginaAReemplazar = paginaConMenorTiempo(paginaAReemplazar, list_get(paginasConFlagEnCero, j));
		}
		nro = paginaAReemplazar->nroDePagina;
		list_destroy(paginasConFlagEnCero);
		bool condicionRemove(st_tablaDePaginas* tabla){
			return paginaAReemplazar->nroDePagina == tabla->nroDePagina;
		}
		for(int j =0; j<listaDeSegmentos->elements_count; j++){
			st_segmento * segmento2= list_get(listaDeSegmentos, j);
			st_tablaDePaginas* pagina = list_remove_by_condition(segmento2->tablaDePaginas, (bool (*)(void *))condicionRemove);
			if(pagina != NULL){
				free(pagina);
			}
		}
		log_info(file_log, "Nro de pagina encontrada %d", paginaAReemplazar->nroDePagina);
		return nro;

	}else{
		log_info(file_log, "No se encontro pagina a reemplazar MEMORIA LLENA");
        return -1;
	}
}



bool tieneFlagEnCero(st_tablaDePaginas* pag){
	return 0 == pag->flagModificado;
}

st_tablaDePaginas* paginaConMenorTiempo(st_tablaDePaginas* paginaSemilla, st_tablaDePaginas* paginaAComparar){

	st_marco* marcoSemilla = list_get(listaDeMarcos, paginaSemilla->nroDePagina);
	st_marco* marcoAComparar = list_get(listaDeMarcos, paginaAComparar->nroDePagina);

	if(marcoSemilla->timestamp > marcoAComparar->timestamp){
		return paginaAComparar;
	} else {
		return paginaSemilla;
	}
}

