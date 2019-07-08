#ifndef MEM_CONSOLE_H
#define MEM_CONSOLE_H

#include <readline/readline.h>
#include <commons/log.h>
#include <stdlib.h>
#include <funcionesCompartidas/API.h>
#include <funcionesCompartidas/listaMetadata.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <funcionesCompartidas/log.h>
#include <pthread.h>
#include "segmentacionPaginada.h"

#include "socketCliente.h"


void console();

void makeCommand(char *command);
void mostrarRespuesta(int respuesta);
void mostrarTabla(st_metadata * meta);
void mostrarTabla(st_metadata * meta);

#endif //MEM_CONSOLE_H
