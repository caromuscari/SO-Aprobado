/*
 * conections.h
 *
 *  Created on: 11 jun. 2019
 *      Author: utnso
 */

#ifndef MEMORIA_CONNECTIONS_H
#define MEMORIA_CONNECTIONS_H

#include <commons/config.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <funcionesCompartidas/log.h>
#include <pthread.h>
#include <funcionesCompartidas/API.h>

void messageAction(header *req, void *buffer, int socketClient);

#endif //MEMORIA_CONNECTIONS_H

