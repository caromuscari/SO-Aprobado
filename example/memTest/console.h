#ifndef MEM_CONSOLE_H
#define MEM_CONSOLE_H

#include <readline/readline.h>
#include <commons/log.h>
#include <stdlib.h>
#include <funcionesCompartidas/API.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <funcionesCompartidas/log.h>
#include "segmentacionPaginada.h"
#include "segmentacionPaginada.c"
//#include <pthread.h>
//#include "conections.h"






void console(st_pagina *punteroTabla);
//void makeCommand(char *command);
#endif //MEM_CONSOLE_H
