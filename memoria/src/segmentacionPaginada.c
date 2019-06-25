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
extern t_condicion* marco;
extern int cantPaginas;

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

void comandoInsert(st_insert* comandoInsert){
	st_segmento* segmentoEncontrado = buscarSegmentoPorNombreTabla(comandoInsert->nameTable);//devuelve el segmento con ese nombre de tabla
	if(segmentoEncontrado){
		log_info(file_log, "Segmento encontrado por comando Insert");
		st_tablaDePaginas* paginaEncontrada = buscarPaginaPorKey(segmentoEncontrado->tablaDePaginas, comandoInsert->key);
		int offset = 0;
		if(paginaEncontrada){
			log_info(file_log, "Pagina encontrada");
			memcpy(paginaEncontrada->pagina + sizeof(double) + sizeof(uint16_t), comandoInsert->value, tamanioValue);
			double timestamp = obtenerMilisegundosDeHoy();
			memcpy(paginaEncontrada->pagina, &timestamp, sizeof(double));
			paginaEncontrada->flagModificado = 1;
			return;
		}
		log_info(file_log, "No se encontro la pagina con esa Key");
	}
	else{
		log_info(file_log, "No se encontro el segmento de esa tabla");
		//creo el segmento
		st_segmento* segmento_nuevo= malloc(sizeof(st_segmento));
		segmento_nuevo->nombreTabla = strdup(comandoInsert->nameTable);
		segmento_nuevo->tablaDePaginas = list_create();

		char* value_mandado = malloc(tamanioValue);


		//int marquitoLibre = buscarMarcoEnMemoria();
		//insertandoElDatoEnElMARCO(marquitoLibre, segmento_nuevo, comandoInsert->key, parametro );

		//Como ya inserte el dato
	}

}

/*
Esta operación incluye los siguientes pasos:
Verifica si existe el segmento de la tabla solicitada y busca en las páginas del mismo si contiene key solicitada.
  Si la contiene, devuelve su valor y finaliza el proceso.
Si no la contiene, envía la solicitud a FileSystem para obtener el valor solicitado y almacenarlo.
Una vez obtenido se debe solicitar una nueva página libre para almacenar el mismo. En caso de no disponer de una página libre,
se debe ejecutar el algoritmo de reemplazo y, en caso de no poder efectuarlo por estar la memoria full,
ejecutar el Journal de la memoria.   */

char* comandoSelect(st_select* comandoSelect){
	st_segmento* segmentoEncontrado;
	segmentoEncontrado = buscarSegmentoPorNombreTabla(comandoSelect->nameTable);//devuelve el segmento con ese nombre de tabla
	if(segmentoEncontrado){
		log_info(file_log, "Segmento encontrado por comando Select");
		st_tablaDePaginas* paginaEncontrada = buscarPaginaPorKey(segmentoEncontrado->tablaDePaginas, comandoSelect->key);
		if(paginaEncontrada){
			char* value = malloc(tamanioValue);
			memcpy(value, paginaEncontrada->pagina+sizeof(double)+sizeof(uint16_t), tamanioValue);
			return value;
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

//double time, uint16_t key, char* value
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
	for(int i = 0; i < cantPaginas; i++){
		if(marco[i] == LIBRE){
			return i;
		}
	}
	int marcoLibre = algoritmoLRU(); //encuentro el marco de la pagina que puedo reemplazar porque se uso hace mas tiempo
	return marcoLibre;
}

int algoritmoLRU(){

	st_segmento* segmento = listaDeSegmentos->head;

//	for(int i = 0, )



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


double obtenerTimeDePag(st_tablaDePaginas* pagina){
	double timestamp;
	memcpy(&timestamp, pagina->pagina, sizeof(double));
	return timestamp;
}



