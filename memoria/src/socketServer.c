#include "socketServer.h"

extern t_log *file_log;

extern t_configuracionMemoria configMemoria;

t_list * listClient;

void messageAction(header *req, void *buffer, int socketClient) {
    header request;
    message *bufferMensaje = NULL;
    size_t sizebuffer = 0;
    int control = 0;

}

void * atenderMensaje(st_client * client){
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
                printf("[+] TimeStamp [%d]\n", insert->timestamp);

                // HACE LA FUNCION INSERT

                //request.letra = 'M';
                //request.codigo = 1;
                //request.sizeData = 0;
                //buffer = createMessage(&request, "");
                //enviar_message(socketClient, bufferMensaje, file_log, &control);

                // DEVOLVER REGISTRO

                break;
            }
            case SELECT:{
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

              break;
            }
        }

}

void *start_server() {
	printf("[+] Starting server.. \n");
	int control = 0;
	int socketServer = makeListenSock(configMemoria.puerto, file_log, &control);
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
