#include "segmentacionPaginada.h"

extern t_list* listaDeMarcos;
extern t_list* listaDeSegmentos;
extern pthread_mutex_t mutexListaMarcos;
extern t_log *file_log;


/*void mostrarPaginasCargadas(){
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
}*/

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
	pthread_mutex_lock(&mutexListaMarcos);
	for(int i = 0; i < listaDeMarcos->elements_count; i++){
		marco = list_get(listaDeMarcos, i);
		if(marco->condicion == LIBRE){
			pthread_mutex_unlock(&mutexListaMarcos);
			log_info(file_log, "Retornando marco libre %d", i);
			return i;
		}
	} pthread_mutex_unlock(&mutexListaMarcos);
	log_info(file_log, "No se encontro marco libre");
	int posMarcoLibre = algoritmoLRU(); //encuentro el marco de la pagina que puedo reemplazar porque se uso hace mas tiempo
	return posMarcoLibre;
}

int algoritmoLRU(){
	log_info(file_log, "Iniciando LRU, size listaSegmento %d", listaDeSegmentos->elements_count);
	t_list* paginasConFlagEnCero = list_create();
	for(int i = 0; i < listaDeSegmentos->elements_count; i++){
		st_segmento* segmento = list_get(listaDeSegmentos, i);
		t_list * pagsAUnir = list_filter(segmento->tablaDePaginas, (void*)tieneFlagEnCero);
		list_add_all(paginasConFlagEnCero, pagsAUnir);
		list_destroy(pagsAUnir);
	}
	if(!list_is_empty(paginasConFlagEnCero)){
		st_tablaDePaginas* paginaAReemplazar = list_fold(paginasConFlagEnCero, paginasConFlagEnCero->head->data, (void*)paginaConMenorTiempo);
		list_destroy(paginasConFlagEnCero);
		return paginaAReemplazar->nroDePagina;

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

