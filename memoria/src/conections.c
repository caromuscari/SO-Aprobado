#include "conections.h"

t_log *file_log;
t_log *log_server;

void messageAction(header *req, void *buffer, int socketClient) {
    header request;
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
            request.letra = 'M';
            request.codigo = 1;
            request.sizeData = 0;
            bufferMensaje = createMessage(&request, "");
            enviar_message(socketClient, bufferMensaje, file_log, &control);
            break;
        }
        case SELECT:{
          st_select * select = deserealizarSelect(buffer);
          printf("[+] We got a SELECT\n");
          printf("[+] Table [%s]\n", select->nameTable);
          printf("[+] Key [%d]\n", select->key);
          request.letra = 'M';
          request.codigo = 1;
          request.sizeData = 0;
          bufferMensaje = createMessage(&request, "");
          enviar_message(socketClient, bufferMensaje, file_log, &control);

          break;
        }
    }
}
