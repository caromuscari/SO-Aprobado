/*
 * segmentacionPaginada.c
 *
 *  Created on: 12 jun. 2019
 *      Author: utnso
 */

#include "segmentacionPaginada.h"

extern t_log *file_log;
extern int tamanioValue;
extern t_list* listaDeMarcos;
extern void *memoriaPrincipal;
extern t_list* listaDeSegmentos;
extern int cantPaginas;
extern int tamanioTotalDePagina;


void inicializarMemoria(){
	listaDeSegmentos = list_create();
	log_info(file_log, "se creo la lista de segmentos");
}

//COMANDO INSERT

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
}

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
	//creo la pagina
	void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));

	memcpy(paginaLibre, &comandoInsert->timestamp, sizeof(double));
	memcpy(paginaLibre + sizeof(double), &comandoInsert->key, sizeof(uint16_t));
	memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), comandoInsert->value, tamanioValue);

	st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
	paginaDeTabla->nroDePagina = posMarcoLibre;
	paginaDeTabla->pagina = paginaLibre;
	paginaDeTabla->flagModificado = 1;

	list_add(segmentoNuevo->tablaDePaginas, paginaDeTabla);

	list_add(listaDeSegmentos, segmentoNuevo);

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
		printf("No se encontro la pagina");
		return NULL;
	}
	printf("No se encontro el segmento");
	return NULL;
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
	return (st_tablaDePaginas*) list_find(tablaDePaginas, paginaConEsaKey);
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

	st_tablaDePaginas* paginaAReemplazar = list_fold(listaDeSegmentos, listaDeSegmentos->head, (void*)paginaConMenorTiempoPorSegmento);

	return paginaAReemplazar->nroDePagina;
}

st_tablaDePaginas* paginaConMenorTiempoPorSegmento(t_list* listaPaginas){
	bool tieneFlagEnCero(st_tablaDePaginas* pagina){
		return 0 == pagina->flagModificado;
	}
	t_list* listaPaginasConFlagEnCero = list_filter(listaPaginas, (void*)tieneFlagEnCero);

	return list_fold(listaPaginasConFlagEnCero, listaPaginasConFlagEnCero->head, (void*)paginaConMenorTiempo);
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

