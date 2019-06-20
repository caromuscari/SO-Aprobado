/*
 * segmentacionPaginada.c
 *
 *  Created on: 12 jun. 2019
 *      Author: utnso
 */

#include "segmentacionPaginada.h"

extern t_log *file_log;

void* memoriaPrincipal;
t_list* listaDeSegmentos;

void inicializarMemoria(){
	memoriaPrincipal = malloc(1000); // aca voy a poner el tamaño segun el largo de value que me pase fs: ts+value+key

	listaDeSegmentos = list_create();
	log_info(file_log, "se creo la lista de segmentos");

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
		log_info(file_log, "segmento encontrado");
		void* paginaEncontrada = buscarPaginaPorKey(segmentoEncontrado->tablaDePaginas, comandoSelect->key);
		if(paginaEncontrada){
			paginaEncontrada += sizeof(double) + sizeof(uint16_t);
			char* value = memcpy(value, paginaEncontrada, sizeof(char*));
			return value;
		}
		return "no se encontro la pag";
	}
	return "no se encontro el segmento";
}

st_segmento* buscarSegmentoPorNombreTabla(char* nombreTabla){
	bool mismoNombreTabla(st_segmento* segmento){
		return 0 == strcmp(segmento->nombreTabla, nombreTabla);
	}
	return list_find(listaDeSegmentos, mismoNombreTabla);
}

//double time, uint16_t key, char* value
void* buscarPaginaPorKey(t_list* tablaDePaginas, uint16_t key){
	bool paginaConEsaKey(void* pagina){
		pagina += sizeof(double);
		uint16_t* keyAComparar = memcpy(keyAComparar, pagina, sizeof(uint16_t));
		return key == *keyAComparar;
	}
	return list_find(tablaDePaginas, paginaConEsaKey);
}






