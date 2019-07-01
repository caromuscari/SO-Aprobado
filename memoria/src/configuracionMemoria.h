/*
 * configuracionMemoria.h
 *
 *  Created on: 15 jun. 2019
 *      Author: utnso
 */

#ifndef CONFIGURACIONMEMORIA_H_
#define CONFIGURACIONMEMORIA_H_

#include <commons/config.h>
#include <commons/log.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

t_log* file_log;
t_config * archivoConfiguracionMemoria;

typedef struct{
	char* puerto;
	char* ipFS;
	char* puertoFS;
	t_list* ipSeeds;
	t_list* puertoSeeds;
	int retardoAccesoMP;
	int retardoAccesoFS;
	int tamanioMemoria;
	int tiempoJournal;
	int tiempoGossiping;
	int nroMemoria;
	int sleep;

}t_configuracionMemoria;

t_configuracionMemoria leerConfiguracion(char* path);

int tamanioArray(void** array);

void hacerFreeArray(void** array);

#endif /* CONFIGURACIONMEMORIA_H_ */
