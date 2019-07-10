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
		log_info(file_log, "No se encontro la pagina con esa Key");

		registro = obtenerSelect(comandoSelect);

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

	list_add(listaDeSegmentos, segmentoNuevo);

	st_marco* marco = list_get(listaDeMarcos, posMarcoLibre);
	marco->condicion = OCUPADO;
	marco->timestamp = obtenerMilisegundosDeHoy();
	mostrarPaginasCargadas();

	return registro;
}





