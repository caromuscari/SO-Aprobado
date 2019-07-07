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
#include <commons/collections/list.h>
#include <commons/string.h>

typedef struct{
	char* PUERTO;
	char* IP_FS;
	char* PUERTO_FS;
	t_list* IP_SEEDS;
	t_list* PUERTO_SEEDS;
	int RETARDO_MEM;
	int RETARDO_FS;
	int TAM_MEM;
	int TIEMPO_JOURNAL;
	int TIEMPO_GOSSIPING;
	int NRO_MEMORIA;
	int SLEEP;

}t_configuracionMemoria;

t_configuracionMemoria * leerConfiguracion(char* path);

#endif /* CONFIGURACIONMEMORIA_H_ */
