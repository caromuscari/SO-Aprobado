#include <stdio.h>
#include <funcionesCompartidas/log.h>
#include "console.h"
#include <commons/collections/list.h>
#include <pthread.h>
#include "segmentacionPaginada.h"
#include "configuracionMemoria.h"
#include "socketServer.h"
#include <funcionesCompartidas/funcionesNET.h>
#include "comandos.h"

t_log *file_log;
t_list *listaDeMarcos;
t_list* listaDeSegmentos;
t_configuracionMemoria *configMemoria;
pthread_t server;
pthread_t gossiping;
int fdFileSystem;
int cantPaginas;
int tamanioValue = 255; //esto me lo va a pasa fs
int tamanioTotalDePagina;
void *memoriaPrincipal;

bool buscarValueMaximo(){
    int control = 0;
    log_info(file_log,"Conectando con FileSysten");
    fdFileSystem = establecerConexion(configMemoria->IP_FS, configMemoria->PUERTO_FS, file_log, &control);
    if(control != 0){
        log_error(file_log,"no se puedo establecer conexion con el FILESYSTEM");
        return false;
    }

    log_info(file_log,"enviando mensaje para solicitar el valor maximo");
    header request;
    request.letra = 'M';
    request.codigo = 1;
    request.sizeData = 1;
    void * paqueteDeMensaje = createMessage(&request," ");
    if(enviar_message(fdFileSystem,paqueteDeMensaje,file_log,&control) < 0){
        log_error(file_log,"no se puedo enviar el mensaje");
        return false;
    }
    free(paqueteDeMensaje);

    log_info(file_log,"esperando respuesta del valor maximo");
    header response;
    void * paqueteDeRespuesta = getMessage(fdFileSystem,&response,&control);
    if(paqueteDeMensaje == NULL){
        log_error(file_log,"erro al recibir el la respuesta");
        return  false;
    }
    //verificar como enviar el mensaje
    tamanioValue = atoi(paqueteDeRespuesta);

    free(paqueteDeRespuesta);
    printf("Tamanio del value: %d",tamanioValue);
    return true;
}

void liberarConfig(t_configuracionMemoria * config){
	free(config->IP_FS);
	free(config->PUERTO);
	list_clean_and_destroy_elements(config->IP_SEEDS, free);
	list_destroy(config->IP_SEEDS);
	free(config->PUERTO_FS);
	list_clean_and_destroy_elements(config->PUERTO_SEEDS,free);
	free(config);
}

int inicializar(char *pathConfig){
    int i;
    file_log = crear_archivo_log("Memoria", false, "./logMemoria");
    log_info(file_log, "cargando el archivo de configuracion");
    configMemoria = leerConfiguracion(pathConfig);
    if (!configMemoria) {
        log_error(file_log, "no se pudo cargar el archivo de configuracion");
        log_destroy(file_log);
        return -1;
    }
    if(!buscarValueMaximo()){
        return -1;
    }
    log_info(file_log, "Inicializar Memoria");
    tamanioTotalDePagina = (sizeof(double) + sizeof(uint16_t) + tamanioValue);
    cantPaginas = configMemoria->TAM_MEM / tamanioTotalDePagina;
    memoriaPrincipal = malloc(configMemoria->TAM_MEM);
    listaDeMarcos = list_create();
    for (i = 0; i < cantPaginas; i++) {
        st_marco *marco = malloc(sizeof(st_marco));
        marco->condicion = LIBRE;
        marco->timestamp = 0;
        list_add(listaDeMarcos, marco);
    }
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
	free(memoriaPrincipal);//Revisar
	list_clean_and_destroy_elements(listaDeMarcos, (void*)free);
	list_destroy(listaDeMarcos);
	list_clean_and_destroy_elements(listaDeSegmentos, (void*)liberarSegmentos);
	list_destroy(listaDeSegmentos);
}

int main(int argc, char *argv[]){
    if (inicializar(argv[1]) < 0) {
        return -1;
    }
    //descomentar cando el File entienda este mensaje
    inicializarMemoria();
    log_info(file_log, "la memoria se inicio correctamente");
    pthread_create(&server, NULL,start_server, NULL);
    pthread_detach(server);
    pthread_create(&gossiping, NULL,pthreadGossping, NULL);
    pthread_detach(gossiping);
    //Falta el join
    console();
    finalizar();
    return 0;
}



