//being a log_server

// console
#include "server.h"
extern t_log *file_log;

t_list * listClient;

typedef struct {
    pthread_t hilo;
    int client;
} st_client;


/*void driver(void *recibido,header request, int client){
	// recibido es el paquete serealizado que nos mandan
    //we have to know who is sending us the message
    if(request.letra == 'K'){

        // once we know that the sender is the kernel, we have to know which operation is he given us
        //and send it to the FL si no esta en memoria

        switch (request.codigo){
            case INSERT:
              printf("[+] I got INSERT.");
              printf("[+] Executing INSERT");
              printf("[+]Sending INSERT to FileSystem");
              messageAction(&request,recibido,client);
              break;

            case SELECT:
              printf("[+] I got SELECT\n");
              printf("[+] Executing SELECT.\n");
              printf("[+]Sending SELECT to FileSystem.\n");
              messageAction(&request,recibido,client);
              break;


        }
    }
} */

char* PORT = "8081";

void * atenderMensaje(st_client * client){
    int control = 0;
    header request;
    void * buffer = getMessage(client->client,&request,&control);
    printf("client = %d\n",client->client);

}

void *start_server() {
	printf("[+] Starting server.. \n");
	int control = 0;
	int socketServer = makeListenSock(PORT, file_log, &control);
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