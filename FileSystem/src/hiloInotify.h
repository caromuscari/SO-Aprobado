//
// Created by utnso on 17/06/19.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <commons/log.h>
#include <commons/config.h>
#include "Funciones.h"
#ifndef FILESYSTEM_HILOINOTIFY_H
#define FILESYSTEM_HILOINOTIFY_H
void hiloinotify(char* rutaConfig);
void cambioConfiguracion(char* rutaConfig);
#endif //FILESYSTEM_HILOINOTIFY_H
