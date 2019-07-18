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
#include "utils.h"
#include "requestMemoria.h"
#include <funcionesCompartidas/codigoMensajes.h>
#include "contracts.h"

void * schedulerMetadata();
TypeCriterio getCriterioByNameTabla(char *nameTable);
void removeTablaByName(char * nameTable);
void addNuevaTabla(st_metadata * metadata);
#endif //KERNEL_THREADMETADATA_H
