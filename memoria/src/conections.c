#include <commons/config.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <funcionesCompartidas/log.h>
#include <pthread.h>
#include <funcionesCompartidas/API.h>

t_log *file_log;
t_log *log_server;

void messageAction(header *req, void *buffer, int socketClient) {
    header response;
    message *bufferMensaje = NULL;
    size_t sizebuffer = 0;
    int control = 0;
    switch (req->codigo) {
        case INSERT: {
            st_insert *insert = desserealizarInsert(buffer);
            printf("[+] We got an INSERT\n");
            printf("[+] Table [%s]\n", insert->nameTable);
            printf("[+] Key [%d]\n", insert->key);
            printf("[+] Value [%s]\n", insert->value);
            printf("[+] TimeStamp [%d]\n", insert->timestamp);
            response.letra = 'M';
            response.codigo = 1;
            response.sizeData = 0;
            bufferMensaje = createMessage(&response, "");
            enviar_message(socketClient, bufferMensaje, file_log, &control);
            break;
        }
        case SELECT:{
          st_select * select = deserealizarSelect(buffer);
          printf("[+] We got a SELECT\n");
          printf("[+] Table [%s]\n", select->nameTable);
          printf("[+] Key [%d]\n", select->key);
          response.letra = 'M';
          response.codigo = 1;
          response.sizeData = 0;
          bufferMensaje = createMessage(&response, "");
          enviar_message(socketClient, bufferMensaje, file_log, &control);

          break;
        }
    }
}
