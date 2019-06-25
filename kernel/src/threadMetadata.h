//
// Created by miguelchauca on 28/04/19.
//

#ifndef KERNEL_THREADMETADATA_H
#define KERNEL_THREADMETADATA_H

#include <funcionesCompartidas/funcionesNET.h>
#include <commons/collections/list.h>
#include <funcionesCompartidas/listaMetadata.h>
#include <funcionesCompartidas/log.h>
#include "load_config.h"

void * schedulerMetadata();
char *getTabla(char *nameTable);
#endif //KERNEL_THREADMETADATA_H
