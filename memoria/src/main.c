#include <stdio.h>
#include <funcionesCompartidas/log.h>
#include "console.h"
#include <commons/collections/list.h>
#include <commons/string.h>
#include <pthread.h>
#include "segmentacionPaginada.h"
#include "configuracionMemoria.h"
#include "socketServer.h"
#include <funcionesCompartidas/funcionesNET.h>
#include "journal.h"

t_log *file_log;
t_list *listaDeMarcos;
t_list* listaDeSegmentos;
t_configuracionMemoria *configMemoria;
pthread_t server;
pthread_t gossiping;
pthread_t journal;
int fdFileSystem;
int cantPaginas;
int tamanioValue;
int tamanioTotalDePagina;
void *memoriaPrincipal;
pthread_mutex_t mutexMemPrinc, mutex, mutexSeeds;

bool buscarValueMaximo(){
    int control = 0;
    log_info(file_log,"Conectando con FileSystem");
    fdFileSystem = establecerConexion(configMemoria->IP_FS, configMemoria->PUERTO_FS, file_log, &control);
    if(control != 0){
        log_error(file_log,"No se pudo establecer conexion con el FileSystem");
        return false;
    }

    log_info(file_log,"Enviando mensaje para solicitar el valor maximo");
    header request;
    request.letra = 'M';
    request.codigo = 1;
    request.sizeData = 1;
    void * paqueteDeMensaje = createMessage(&request," ");
    if(enviar_message(fdFileSystem,paqueteDeMensaje,file_log,&control) < 0){
        log_error(file_log,"No se pudo enviar el mensaje");
        return false;
    }
    free(paqueteDeMensaje);

    log_info(file_log,"Esperando respuesta del valor maximo");
    header response;
    char * paqueteDeRespuesta = getMessage(fdFileSystem,&response,&control);
    if(paqueteDeRespuesta == NULL){
        log_error(file_log,"Error al recibir la respuesta");
        return  false;
    }
    char* intermedia = strdup(paqueteDeRespuesta);
    //verificar como enviar el mensaje
    tamanioValue = atoi(intermedia);

    free(intermedia);
    free(paqueteDeRespuesta);
    printf("Tamanio del value: %d\n",tamanioValue);
    return true;
}

void liberarConfig(t_configuracionMemoria * config){
	free(config->IP_FS);
	free(config->PUERTO);
	list_clean_and_destroy_elements(config->IP_SEEDS, free);
	list_destroy(config->IP_SEEDS);
	free(config->PUERTO_FS);
	list_clean_and_destroy_elements(config->PUERTO_SEEDS,free);
	list_destroy(config->PUERTO_SEEDS);
	free(config);
}

int inicializar(char *pathConfig){
    int i;
    configMemoria = leerConfiguracion(pathConfig);
    char* nombreLog = strdup("./logMemoria-");
    string_append(&nombreLog, string_itoa(configMemoria->NRO_MEMORIA));
    string_append(&nombreLog, ".log");
    file_log = crear_archivo_log("Memoria", false, nombreLog);
    log_info(file_log, "Archivo de configuracion cargado");
    if (!configMemoria) {
        log_error(file_log, "No se pudo cargar el archivo de configuracion");
        log_destroy(file_log);
        return -1;
    }
    if(!buscarValueMaximo()){
    	printf("No me pude conectar al FileSystem\n");
    	log_info(file_log, "No me pude conectar al FileSystem");
        return -1;
    }
    log_info(file_log, "Inicializando Memoria");
    tamanioTotalDePagina = (sizeof(double) + sizeof(uint16_t) + tamanioValue);
    cantPaginas = configMemoria->TAM_MEM / tamanioTotalDePagina;
    memoriaPrincipal = malloc(configMemoria->TAM_MEM);
    listaDeSegmentos = list_create();
    listaDeMarcos = list_create();
    for (i = 0; i < cantPaginas; i++) {
        st_marco *marco = malloc(sizeof(st_marco));
        marco->condicion = LIBRE;
        marco->timestamp = 0;
        list_add(listaDeMarcos, marco);
    }
    pthread_mutex_init(&mutexMemPrinc, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexSeeds, NULL);
    return 0;
}

void liberarSegmentos(st_segmento* segmento){
	free(segmento->nombreTabla);
	list_clean_and_destroy_elements(segmento->tablaDePaginas, (void*)free);
	list_destroy(segmento->tablaDePaginas);
	free(segmento);
}

void finalizar(){
	pthread_cancel(server);
	liberar_log(file_log);
	liberarConfig(configMemoria);
	free(memoriaPrincipal);
	list_clean_and_destroy_elements(listaDeMarcos, (void*)free);
	list_destroy(listaDeMarcos);
	list_clean_and_destroy_elements(listaDeSegmentos, (void*)liberarSegmentos);
	list_destroy(listaDeSegmentos);
}

int main(int argc, char *argv[]){
    if (inicializar(argv[1]) < 0) {
        return -1;
    }
    log_info(file_log, "La memoria se inicio correctamente");
    pthread_create(&journal, NULL, hiloJournal, NULL);
    pthread_detach(journal);
    pthread_create(&server, NULL,start_server, NULL);
    pthread_detach(server);
    pthread_create(&gossiping, NULL,pthreadGossping, NULL);
    pthread_detach(gossiping);
    console();
    finalizar();
    return 0;
}



