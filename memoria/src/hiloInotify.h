/*
 * hiloInotify.h
 *
 *  Created on: 3 ago. 2019
 *      Author: utnso
 */

#ifndef SRC_HILOINOTIFY_H_
#define SRC_HILOINOTIFY_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>

void hiloinotify(char* rutaConfig);

void cambioConfiguracion(char* rutaConfig);

int getRetardoMem();

int getRetardoFile();

#endif /* SRC_HILOINOTIFY_H_ */
