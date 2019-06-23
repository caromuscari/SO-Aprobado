#include <stdio.h>
#include <funcionesCompartidas/log.h>
#include "console.h"
#include "server.h"
#include <commons/collections/list.h>
#include <pthread.h>
#include "segmentacionPaginada.h"
#include "configuracionMemoria.h"

#define IPFILESYSTEM "192.168.0.21"
#define PORTFILESYSTEM "8080"

#define MYPORT "8081"

t_log *file_log;

char *IP_FS = IPFILESYSTEM;
char *PORT_FS = PORTFILESYSTEM;

int tamanioMemoria;
t_condicion* marco;
int cantPaginas;
int tamanioValue = 4; //esto me lo va a pasa fs

int main(int argc, char const *argv[]) {

	pthread_t server;
	pthread_t client;

    file_log = crear_archivo_log("Memoria", true,"./logC");

	t_configuracionMemoria configMemoria = leerConfiguracion("/home/miguelchauca/Documents/Operativo/workSpace/tp-2019-1c-misc/memoria/memoriaConf.config");
	int tamanioMemoria = configMemoria.tamanioMemoria;

	int cantPaginas = tamanioMemoria / (sizeof(double) + sizeof(uint16_t) + tamanioValue);
	t_condicion* marco = malloc(cantPaginas);

	inicializarMemoria();
	log_info(file_log, "la memoria se inicio correctamente");

    pthread_create(&server,NULL, &start_server, MYPORT);
    //pthread_create(&client,NULL, &connectToSeeds,(void *) totalSeeds);

//    pthread_join(server,NULL);
//    //pthread_join(client,NULL);

	console();

	return 0;
}





