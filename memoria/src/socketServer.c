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
        log_error(file_log, "error al obtener el mensaje");
        close(*fdClient);
        pthread_exit(NULL);
    }
    switch (request.codigo) {
    	case INSERT: {
    		st_insert *insert = desserealizarInsert(paqueteDeRespuesta);
    		printf("[+] We got an INSERT\n");
    		printf("[+] Table [%s]\n", insert->nameTable);
    		printf("[+] Key [%d]\n", insert->key);
    		printf("[+] Value [%s]\n", insert->value);
    		printf("[+] TimeStamp [%f]\n", insert->timestamp);

    		// HACE LA FUNCION INSERT

    		//request.letra = 'M';
    		//request.codigo = 1;
    		//request.sizeData = 0;
    		//buffer = createMessage(&request, "");
    		//enviar_message(socketClient, bufferMensaje, file_log, &control);

    		// DEVOLVER REGISTRO

    		destroyInsert(insert);
    		break;
    	}
        case SELECT: {
            st_select * select = deserealizarSelect(paqueteDeRespuesta);
            printf("[+] We got a SELECT\n");
            printf("[+] Table [%s]\n", select->nameTable);
            printf("[+] Key [%d]\n", select->key);

            // IDEM INSERT

            //request.letra = 'M';
            //request.codigo = 1;
            //request.sizeData = 0;
            //bufferMensaje = createMessage(&request, "");
            //enviar_message(socketClient, bufferMensaje, file_log, &control);

            destoySelect(select);
            break;
        }
    	case CREATE:{
            st_create * create = deserealizarCreate(paqueteDeRespuesta);
            int respuesta;
            printf("We got a CREATE\n");

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
            printf("We got a DROP");

            respuesta = mandarDrop(drop);

            buffer = strdup("");

            if(respuesta == 9) enviarRespuesta(7, buffer, *fdClient, &control, sizeof(buffer));
            else enviarRespuesta(8, buffer, *fdClient, &control, sizeof(buffer));

            destroyDrop(drop);
            free(buffer);
            break;
    	}
    	case DESCRIBE:{
    		int respuesta;
    		st_metadata * meta;
    		size_t size;
            st_describe * describe = deserealizarDescribe(paqueteDeRespuesta);
            printf("We got a DESCRIBE");

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
    		printf("We got a DESCRIBE Global");
    		//mock
//            t_list *listametadata = list_create();
 //           cargarLista(listametadata);
   //         buffer = serealizarListaMetaData(listametadata, &size);
     //       enviarRespuesta(13, buffer, *fdClient, &control,size);

    		respuesta = mandarDescribeGlobal(&buffer,&size);
    		//Cambiar Numeros
    		if(respuesta == 13){
    			enviarRespuesta(13, buffer, *fdClient, &control,size);
    		}else{
    			enviarRespuesta(20, NULL, *fdClient, &control,size);
    		}
    		if(buffer) free(buffer);
    		break;
    	}
        case BUSCARTABLAGOSSIPING: {
            void *paqueteLista = devolverListaMemoria(&sizePaqueteRes);
            enviarRespuesta(DEVOLVERTABLAGOSSIPING,paqueteLista,*fdClient,&control,sizePaqueteRes);
            break;
        }
    	case JOURNAL:
    		break;
    }

    free(paqueteDeRespuesta);
    close(*fdClient);
    pthread_exit(NULL);
}

void * start_server() {
	int control = 0;
	int socketServer = makeListenSock(configMemoria->PUERTO, file_log, &control);
	int  * fdClient;
    pthread_t nuevoCliente;
	while (true){
        fdClient = malloc(sizeof(int));
        *fdClient = aceptar_conexion(socketServer, file_log, &control);
        if (control != 0) {
            log_error(file_log, "no se puede aceptar la conexion");
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
	    log_error(file_log,"no se puedo enviar una respuesta al kernel");
	}

	free(head);
	free(mensaje->buffer);
	free(mensaje);
}
