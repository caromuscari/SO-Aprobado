#ifndef MEM_SERVER_H
#define MEM_SERVER_H


#include <commons/config.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <funcionesCompartidas/log.h>
#include <pthread.h>
#include <funcionesCompartidas/API.h>
#include <readline/readline.h>
#include <commons/log.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include "conections.c"

void *start_server(char *PORT);
void driver(void *recibido,header request);

#endif //MEM_SERVER_H
