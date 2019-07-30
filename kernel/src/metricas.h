//
// Created by miguel on 27/7/19.
//

#ifndef KERNEL_METRICAS_H
#define KERNEL_METRICAS_H

#include "threadPoolMemoria.h"
#include "contracts.h"
#include <funcionesCompartidas/API.h>
#include <funcionesCompartidas/log.h>

void calcularMetricas(bool printConsole);
void pthreadLogMetricas();
#endif //KERNEL_METRICAS_H
