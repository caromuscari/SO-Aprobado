#include "socketServer.h"
#include "request.h"

extern t_log *file_log;
extern t_configuracionMemoria * configMemoria;
#include <funcionesCompartidas/API.h>
#include <funcionesCompartidas/listaMetadata.h>
#include <commons/collections/list.h>

void cargarLista(t_list *listaMetaData) {
    st_metadata *metadata = malloc(sizeof(st_metadata));
    metadata->nameTable = strdup("TABLA_A");
    metadata->consistency = strdup("SC");
    metadata->partitions = 4;
    metadata->compaction_time = 5000;
    list_add(listaMetaData, metadata);
    metadata = malloc(sizeof(st_metadata));
    metadata->nameTable = strdup("TABLA_B");
    metadata->consistency = strdup("EC");
    metadata->partitions = 3;
    metadata->compaction_time = 6000;
    list_add(listaMetaData, metadata);
    metadata = malloc(sizeof(st_metadata));
    metadata->nameTable = strdup("TABLA_C");
    metadata->consistency = strdup("SHC");
    metadata->partitions = 5;
    metadata->compaction_time = 10000;
    list_add(listaMetaData, metadata);
}

void * atenderMensaje(int * fdClient){
    int control = 0;
    header request;
    header response;
    size_t sizePaqueteRes = 0;
    void * buffer = NULL;
    void * paqueteDeRespuesta = getMessage(*fdClient,&request,&control);
    if (control < 0) {
        log_error(file_log, "Error al obtener el mensaje\n");
        close(*fdClient);
        pthread_exit(NULL);
    }
    switch (request.codigo) {
    	case INSERT: {
    		st_insert *insert = desserealizarInsert(paqueteDeRespuesta);
    		printf("El comando es un Insert\n");
    		printf("Table [%s]\n", insert->nameTable);
    		printf("Key [%d]\n", insert->key);
    		printf("Value [%s]\n", insert->value);

    		int resultado = comandoInsert(insert);
    		if(resultado == -1){
    			enviarRespuesta(2, NULL, *fdClient, &control, 0);
    		}
    		if(resultado == -2){
    			//memoria full
    			enviarRespuesta(3, NULL, *fdClient, &control, 0);
    		}else {
    			enviarRespuesta(1, NULL, *fdClient, &control, 0);
    		}
    		destroyInsert(insert);
    		break;
    	}
        case SELECT: {
            st_select * select = deserealizarSelect(paqueteDeRespuesta);
            printf("El comando es un Insert\n");
            st_registro* registro = comandoSelect(select);
            if(registro){
            	buffer = serealizarRegistro(registro, &sizePaqueteRes);
            	enviarRespuesta(1, buffer, *fdClient, &control, sizePaqueteRes);
            } else {
            	 enviarRespuesta(2, NULL, *fdClient, &control, 0);
            }
            destoySelect(select);
            break;
        }
    	case CREATE:{
            st_create * create = deserealizarCreate(paqueteDeRespuesta);
            int respuesta;
            printf("El comando es un Create\n");

            respuesta = mandarCreate(create);

            buffer = strdup("");

            if(respuesta == 8) enviarRespuesta(7, buffer, *fdClient, &control, sizeof(buffer));
            else enviarRespuesta(8, buffer, *fdClient, &control, sizeof(buffer));

            destroyCreate(create);
            free(buffer);
            break;
    	}
    	case DROP:{
            st_drop * drop = deserealizarDrop(paqueteDeRespuesta);
            int respuesta;
            printf("El comando es un Drop\n");

            respuesta = mandarDrop(drop);

            if(respuesta == 9) enviarRespuesta(1, NULL, *fdClient, &control, 0);
            else enviarRespuesta(2, NULL, *fdClient, &control, 0);

            destroyDrop(drop);
            break;
    	}
    	case DESCRIBE:{
    		int respuesta;
    		st_metadata * meta;
    		size_t size;
            st_describe * describe = deserealizarDescribe(paqueteDeRespuesta);
            printf("El comando es un Describe \n");

            respuesta = mandarDescribe(describe,&meta);

            //Cambiar Numeros
            if(respuesta == 15){
            	buffer = serealizarMetaData(meta,&size);
            	enviarRespuesta(15,buffer,*fdClient,&control,size);
            	destroyMetaData(meta);
            }else{
            	buffer = strdup("");
            	enviarRespuesta(15,buffer,*fdClient,&control,sizeof(buffer));
            }

            destroyDescribe(describe);
            free(buffer);
            break;
    	}
    	case DESCRIBEGLOBAL:{
    		int respuesta;
    		size_t size = 0;
    		printf("El comando es un Describe Global\n");
    		//mock
            t_list *listametadata = list_create();
            cargarLista(listametadata);
            buffer = serealizarListaMetaData(listametadata, &size);
            enviarRespuesta(13, buffer, *fdClient, &control,size);
            //
    		//respuesta = mandarDescribeGlobal(&buffer,&size);
    		//Cambiar Numeros
//    		if(respuesta == 13){
//    			enviarRespuesta(13, buffer, *fdClient, &control,size);
//    		}else{
//    			enviarRespuesta(20, NULL, *fdClient, &control,size);
//    		}
    		if(buffer) free(buffer);
    		break;
    	}
        case BUSCARTABLAGOSSIPING: {
            void *paqueteLista = devolverListaMemoria(&sizePaqueteRes);
            enviarRespuesta(DEVOLVERTABLAGOSSIPING,paqueteLista,*fdClient,&control,sizePaqueteRes);
            break;
        }
    	case JOURNAL:{
    		printf("Realizando Journal");
    		int respuesta = comandoJournal();
    		if(respuesta == -1){
    			enviarRespuesta(2, NULL, *fdClient, &control, 0);
    		} else {
    			enviarRespuesta(1, NULL, *fdClient, &control, 0);
    		}
    		break;
    	}
    }

    free(paqueteDeRespuesta);
    close(*fdClient);
    pthread_exit(NULL);
}

void * start_server() {
	int control = 0;
	int socketServer = makeListenSock(configMemoria->PUERTO, file_log, &control);
	if(control != 0){
		pthread_exit(NULL);
	}
	int  * fdClient;
    pthread_t nuevoCliente;
	while (true){
        fdClient = malloc(sizeof(int));
        *fdClient = aceptar_conexion(socketServer, file_log, &control);
        if (control != 0) {
            log_error(file_log, "No se puede aceptar la conexion");
            continue;
        }
        pthread_create(&nuevoCliente,NULL,(void *)atenderMensaje,fdClient);
        pthread_detach(nuevoCliente);
	}
}

void enviarRespuesta(int codigo, char * buffer, int socketC, int * status, size_t tam){

	header * head = malloc(sizeof(header));

	head->letra = 'M';
	head->codigo = codigo;
	head->sizeData = tam;

	message * mensaje = createMessage(head, buffer);

	enviar_message(socketC, mensaje, file_log, status);
	if(*status != 0){
	    log_error(file_log,"No se pudo enviar una respuesta al kernel");
	}

	free(head);
	free(mensaje->buffer);
	free(mensaje);
}
