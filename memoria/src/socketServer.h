#ifndef MEM_SERVER_H
#define MEM_SERVER_H


#include <commons/config.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <funcionesCompartidas/log.h>
#include <pthread.h>
#include <readline/readline.h>
#include <commons/log.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <funcionesCompartidas/codigoMensajes.h>
#include "gossiping.h"
#include "socketCliente.h"

void * start_server();
void enviarRespuesta(int codigo, char * buffer, int socketC, int * status, size_t tam);


#endif //MEM_SERVER_H
