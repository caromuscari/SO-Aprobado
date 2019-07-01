#include <stdio.h>
#include <funcionesCompartidas/log.h>
#include "console.h"
#include <commons/collections/list.h>
#include <pthread.h>
#include "segmentacionPaginada.h"
#include "configuracionMemoria.h"
#include "socketServer.h"


t_log *file_log;

int tamanioMemoria;
t_list* listaDeMarcos;
int cantPaginas;
int tamanioValue = 4; //esto me lo va a pasa fs

t_configuracionMemoria configMemoria;

int main(int argc, char const *argv[]) {

	pthread_t server;
	pthread_t client;

    file_log = crear_archivo_log("Memoria", true,"./logC");

	configMemoria = leerConfiguracion("/home/utnso/workspace/tp-2019-1c-misc/memoria/memoriaConf.config");
	int tamanioMemoria = configMemoria.tamanioMemoria;

	int cantPaginas = tamanioMemoria / (sizeof(double) + sizeof(uint16_t) + tamanioValue);

	listaDeMarcos = list_create();
	for(int i = 0; i<cantPaginas; i++){
		st_marco* marco= malloc(sizeof(st_marco));
		marco->condicion = LIBRE;
		marco->timestamp = 0;
		list_add(listaDeMarcos, marco);
	}

	printf("%d", listaDeMarcos->elements_count);



	inicializarMemoria();
	log_info(file_log, "la memoria se inicio correctamente");

    pthread_create(&server,NULL, &start_server, NULL);

	console();

	return 0;
}





