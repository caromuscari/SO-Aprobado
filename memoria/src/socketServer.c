#include "socketServer.h"

extern t_log *file_log;
extern t_configuracionMemoria * configMemoria;
#include <funcionesCompartidas/API.h>

t_list * listClient;

void atenderMensaje(st_client * client){
    int control = 0;
    header request;
    void * paqueteDeRespuesta = getMessage(client->client,&request,&control);
    printf("client = %d\n",client->client);
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
            printf("We got a CREATE\n");

            //Hacer create

            destroyCreate(create);
            break;
    	}
    	case DROP:{
            st_drop * drop = deserealizarDrop(paqueteDeRespuesta);
            printf("We got a DROP");

            //Hacer drop

            destroyDrop(drop);
            break;
    	}
    	case DESCRIBE:{
            st_describe * describe = deserealizarDescribe(paqueteDeRespuesta);
            printf("We got a DESCRIBE");

            //Hacer describe

            destroyDescribe(describe);
            break;
    	}
    	case JOURNAL:
    		break;
    }

    free(paqueteDeRespuesta);
    free(client);

    pthread_exit(NULL);
}

void *start_server() {
	printf("[+] Starting server.. \n");
	int control = 0;
	int socketServer = makeListenSock(configMemoria->PUERTO, file_log, &control);
	int client;
	st_client * newClient;
	while (true){
        client = aceptar_conexion(socketServer, file_log, &control);
        newClient = malloc(sizeof(newClient));
        newClient->client = client;
        pthread_create(&newClient->hilo,NULL,(void *)atenderMensaje,newClient);
        pthread_detach(newClient->hilo);
	}
}
