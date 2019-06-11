//being a log_server

// console
#include "server.h"


void driver(void *recibido,header request){
    //we have to know who is seending us the message
    if(request.letra == 'K'){

        // once we know that the sender is de kernel, we have to know which operation is he given us
        //and send it to the FL

        switch (request.codigo){
            case INSERT:{
              printf("[+] I got INSERT.");
              printf("[+] Executing INSERT");
              printf("[+]Sending INSERT to FileSystem";
              messageAction(INSERT,recibido,client);
              break;
            }
            case SELECT:{
              printf("[+] I got SELECT\n");
              printf("[+] Executing SELECT.\n");
              printf("[+]Sending SELECT to FileSystem.\n");
              messageAction(SELECT,recibido,client);
              break;
            }
        }
    }
}


void *start_server(char PORT){

  printf("[+] Starting server.. \n");

    int control = 0;
    int socketServer = makeListenSock(PORT,log_server, &control);

    int client = aceptar_conexion(socketServer,log_server,&control);

    if(control == 0){
      log_info(log_server,"[+] So far so good..\n");
    }

    printf("[+] Memory [ID: %d] connected\n", client );

    while(true){
      header request;

      void * recibido;
      recibido = getMessage(client, &request, &control);

      if(recibido == NULL){
         perror("[-] It's an empy package :( \n");
         return -1;
      }

      driver(recibido,client);
      free(recibido);
    }

    fclose(client);

  }
