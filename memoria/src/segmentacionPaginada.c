/*
 * segmentacionPaginada.c
 *
 *  Created on: 12 jun. 2019
 *      Author: utnso
 */

#include "segmentacionPaginada.h"

extern t_log *file_log;
extern int tamanioMemoria;
extern int tamanioValue;
extern t_list* listaDeMarcos;

void* memoriaPrincipal;
t_list* listaDeSegmentos;


void inicializarMemoria(){
	memoriaPrincipal = malloc(tamanioMemoria);

	listaDeSegmentos = list_create();
	log_info(file_log, "se creo la lista de segmentos");

}
/* INSERT [NOMBRE_TABLA] [KEY] “[VALUE]”
	Esta operación incluye los siguientes pasos:
1. Verifica si existe el segmento de la tabla en la memoria principal. De existir, busca en sus páginas si contiene la key
solicitada y de contenerla actualiza el valor insertando el Timestamp actual. En caso que no contenga la Key, se solicita
una nueva página para almacenar la misma. Se deberá tener en cuenta que si no se disponen de páginas libres aplicar el
algoritmo de reemplazo y en caso de que la memoria se encuentre full iniciar el proceso Journal.
2. En caso que no se encuentre el segmento en memoria principal, se creará y se agregará la nueva Key con el Timestamp
actual, junto con el nombre de la tabla en el segmento. Para esto se debe generar el nuevo segmento y solicitar una nueva
página (aplicando para este último la misma lógica que el punto anterior).
*/

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

		insertarDatosEnPagina(paginaLibre, comandoInsert);

		st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
		paginaDeTabla->nroDePagina = posMarcoLibre;
		paginaDeTabla->pagina = paginaLibre;
		paginaDeTabla->flagModificado = 1;

		list_add(segmentoEncontrado->tablaDePaginas, paginaDeTabla);

		st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
		marco->condicion = OCUPADO;

		return 0;
	}
	log_info(file_log, "No se encontro el segmento de esa tabla");
	//creo el segmento
	st_segmento* segmentoNuevo= malloc(sizeof(st_segmento));

	segmentoNuevo->nombreTabla = strdup(comandoInsert->nameTable);
	segmentoNuevo->tablaDePaginas = list_create();

	int posMarcoLibre = buscarMarcoLibre();

	void* paginaLibre = memoriaPrincipal + (posMarcoLibre * (sizeof(double) + sizeof(uint16_t) + tamanioValue));

	insertarDatosEnPagina(paginaLibre, comandoInsert);

	st_tablaDePaginas* paginaDeTabla = malloc(sizeof(st_tablaDePaginas));
	paginaDeTabla->nroDePagina = posMarcoLibre;
	paginaDeTabla->pagina = paginaLibre;
	paginaDeTabla->flagModificado = 1;

	list_add(segmentoNuevo->tablaDePaginas, paginaDeTabla);

	list_add(listaDeSegmentos, segmentoNuevo);

	st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
	marco->condicion = OCUPADO;

	return 0;
}

insertarDatosEnPagina(void* paginaLibre, st_insert * comandoInsert){

	memcpy(paginaLibre, &comandoInsert->timestamp, sizeof(double));
	memcpy(paginaLibre + sizeof(double), &comandoInsert->key, sizeof(uint16_t));
	memcpy(paginaLibre + sizeof(double) + sizeof(uint16_t), comandoInsert->value, tamanioValue);
}


/* SELECT
Verifica si existe el segmento de la tabla solicitada y busca en las páginas del mismo si contiene key solicitada.
  Si la contiene, devuelve su valor y finaliza el proceso.
Si no la contiene, envía la solicitud a FileSystem para obtener el valor solicitado y almacenarlo.
Una vez obtenido se debe solicitar una nueva página libre para almacenar el mismo. En caso de no disponer de una página libre,
se debe ejecutar el algoritmo de reemplazo y, en caso de no poder efectuarlo por estar la memoria full,
ejecutar el Journal de la memoria.   */

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
		return "No se encontro la pagina";
	}
	return "No se encontro el segmento";
}


st_segmento* buscarSegmentoPorNombreTabla(char* nombreTabla){
	bool mismoNombreTabla(st_segmento* segmento){
		return 0 == strcmp(segmento->nombreTabla, nombreTabla);
	}
	return (st_segmento*)list_find(listaDeSegmentos, mismoNombreTabla);
}


st_tablaDePaginas* buscarPaginaPorKey(t_list* tablaDePaginas, uint16_t key){
	bool paginaConEsaKey(void* pagina){
		pagina += sizeof(double);
		uint16_t keyAComparar;
		memcpy(&keyAComparar, pagina, sizeof(uint16_t));
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

	st_segmento* segmento = listaDeSegmentos->head;

//	for(int i = 0, )


	return 0; //arreglar
}

//void* list_get(t_list *self, int index)
//int list_count_satisfying(t_list* self, bool(*condition)(void*))

st_tablaDePaginas* paginaConTiempoMenorPorSegmento(t_list* listaPaginas){
	bool tieneFlagEnCero(st_tablaDePaginas* pagina){
		return 0 == pagina->flagModificado;
	}
	t_list* listaPaginasConFlagEnCero = list_filter(listaPaginas, tieneFlagEnCero);

	st_tablaDePaginas* paginaConMenorTime;
//
//	for(int i = 1; i< list_size(listaPaginasConFlagEnCero) -1; i++){
//
//		}
//	}
}

