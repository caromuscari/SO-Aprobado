#include "segmentacionPaginada.h"

extern int tamanioValue;
extern t_list* listaDeMarcos;
extern void *memoriaPrincipal;
extern t_list* listaDeSegmentos;
extern int cantPaginas;
extern int tamanioTotalDePagina;
extern t_log *file_log;


void mostrarPaginasCargadas(){
    int i,offset = 0;
    uint16_t key;
    double timeStamp;
    char * value = malloc(tamanioValue);
    for (i = 0; i < cantPaginas; ++i) {
        offset = tamanioTotalDePagina * i;
        memcpy(&timeStamp, (memoriaPrincipal + offset), sizeof(double));
        memcpy(&key,(memoriaPrincipal + offset + sizeof(double)), sizeof(uint16_t));
        memcpy(value,(memoriaPrincipal + offset + sizeof(double) + sizeof(uint16_t)), tamanioValue);
        printf("pagina [%d]-------\n",i);
        printf("%f\n",timeStamp);
        printf("%d\n",key);
        printf("%s\n",value);
    }
    free(value);
}

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
	for(int i = 0; i < listaDeMarcos->elements_count; i++){
		marco = list_get(listaDeMarcos, i);
		if(marco->condicion == LIBRE){
		return i;
		}
	}
	int posMarcoLibre = algoritmoLRU(); //encuentro el marco de la pagina que puedo reemplazar porque se uso hace mas tiempo
	return posMarcoLibre;
}

int algoritmoLRU(){

	st_tablaDePaginas* paginaAReemplazar = list_fold(listaDeSegmentos, listaDeSegmentos->head->data, (void*)paginaConMenorTiempoPorSegmento);

	if(paginaAReemplazar){
        return paginaAReemplazar->nroDePagina;
	}else{
        return -1;
	}
}

st_tablaDePaginas* paginaConMenorTiempoPorSegmento(st_segmento * stSegmento){
	bool tieneFlagEnCero(st_tablaDePaginas* pagina){
		return 0 == pagina->flagModificado;
	}
	t_list* listaPaginasConFlagEnCero = list_filter(stSegmento->tablaDePaginas, (void*)tieneFlagEnCero);

	if(!list_is_empty(listaPaginasConFlagEnCero)){
        return list_fold(listaPaginasConFlagEnCero, listaPaginasConFlagEnCero->head->data, (void*)paginaConMenorTiempo);
	}
    return NULL;
}

st_tablaDePaginas* paginaConMenorTiempo(st_tablaDePaginas* paginaSemilla, st_tablaDePaginas* paginaAComparar){

	st_marco* marcoSemilla = list_get(listaDeMarcos, paginaSemilla->nroDePagina);
	st_marco* marcoAComparar = list_get(listaDeMarcos, paginaAComparar->nroDePagina);

	if(marcoSemilla->timestamp > marcoAComparar->timestamp){
		free(marcoAComparar);
		free(marcoSemilla);
		return paginaAComparar;
	} else {
		free(marcoAComparar);
		free(marcoSemilla);
		return paginaSemilla;
	}
}

int removerSegmentoPorNombrePagina(char* nombreTabla){
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
	        list_remove(listaDeSegmentos, segmentoEncontrado->nroSegmento);
	        for(int i = segmentoEncontrado->nroSegmento; i < list_size(listaDeSegmentos); i++){
	            st_segmento* segmento = list_get(listaDeSegmentos, i);
	            int nro = segmento->nroSegmento;
	            segmento->nroSegmento = nro - 1;
	        }
	        return OK;
	    } else {
	    	log_info(file_log, "No se encontro el segmento por Drop");
	    	return NOOK;
	    }
}

