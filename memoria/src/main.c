#include <stdio.h>
#include <funcionesCompartidas/log.h>
#include "console.h"
#include <commons/collections/list.h>
#include <pthread.h>
#include "segmentacionPaginada.h"
#include "configuracionMemoria.h"
#include "socketServer.h"
#include <funcionesCompartidas/funcionesNET.h>

t_log *file_log;
t_list *listaDeMarcos;
t_configuracionMemoria *configMemoria;
int fdFileSystem;
int cantPaginas;
int tamanioValue = 4; //esto me lo va a pasa fs
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
    return true;
}

int inicializar(char *pathConfig){
    int i;
    file_log = crear_archivo_log("Memoria", true, "./logMemoria");
    log_info(file_log, "cargando el archivo de configuracion");
    configMemoria = leerConfiguracion(pathConfig);
    if (!configMemoria) {
        log_error(file_log, "no se pudo cargar el archivo de configuracion");
        log_destroy(file_log);
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

int main(int argc, char *argv[]){
    if (inicializar(argv[1]) < 0) {
        return -1;
    }
    //descomentar cando el File entienda este mensaje
    //if(!buscarValueMaximo()){
    //    return -1;
    //}
    pthread_t server;
    inicializarMemoria();
    log_info(file_log, "la memoria se inicio correctamente");
    pthread_create(&server, NULL, &start_server, NULL);
    //Falta el join
    console();
    return 0;
}





