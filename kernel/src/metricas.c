//
// Created by miguel on 27/7/19.
//

#include "metricas.h"
extern t_log *file_log;

t_list * getHistoryByCriterio(t_list * listHistory, TypeCriterio criterio){
    t_list * listMemoria = list_create();
    st_history_request * historyRequest;
    int i;
    for (i = 0; i < listHistory->elements_count ; ++i) {
        historyRequest = list_get(listHistory,i);
        if(criterio == historyRequest->tipoCriterio){
            list_add(listMemoria,historyRequest);
        }
    }
    return  listMemoria;
}

t_list * getHistoryByOperation(t_list * listHistory, enum OPERACION operacion){
    t_list * listMemoria = list_create();
    st_history_request * historyRequest;
    int i;
    for (i = 0; i < listHistory->elements_count ; ++i) {
        historyRequest = list_get(listHistory,i);
        if(operacion == historyRequest->operacion){
            list_add(listMemoria,historyRequest);
        }
    }
    return  listMemoria;
}

double promedioBYOperation(t_list * listHistory, TypeCriterio criterio,enum OPERACION operacion){
    t_list * listaByCriterio = getHistoryByCriterio(listHistory,criterio);
    t_list * listaByOperacion = getHistoryByOperation(listaByCriterio, operacion);
    int i;
    double promedio = 0;
    st_history_request * historyRequest = NULL;
    double timeTotal = 0;
    for (i = 0; i < listaByOperacion->elements_count ; ++i) {
        historyRequest = list_get(listaByOperacion,i);
        timeTotal += (historyRequest->endTime - historyRequest->startTime);
    }
    if(listaByOperacion->elements_count > 0){
        promedio = timeTotal / listaByOperacion->elements_count;
    }
    list_destroy(listaByCriterio);
    list_destroy(listaByOperacion);
    return  promedio;
}

int getTotalBYOperacion(t_list * listHistory, TypeCriterio criterio,enum OPERACION operacion){
    t_list * listaByCriterio = getHistoryByCriterio(listHistory,criterio);
    t_list * listaByOperacion = getHistoryByOperation(listaByCriterio, operacion);
    int total = listaByOperacion->elements_count;
    list_destroy(listaByCriterio);
    list_destroy(listaByOperacion);
    return  total;
}

void destroyMetricaMemoria(st_metrica_memoria * metricaMemoria){
    list_iterate(metricaMemoria->tipos, free);
    list_destroy(metricaMemoria->tipos);
    free(metricaMemoria);
}

void calcularMetricas(bool printConsole){
    double now = obtenerMilisegundosDeHoy();
    int i,j,k;
    st_metrica_memoria * metricaMemoria = NULL;
    TypeCriterio * criterio;
    enum OPERACION arrayOperacion[] = {INSERT,SELECT};
    t_list * listaMemorias = getHistoryByRange((now-30000),now);
    double promerio = 0;
    int total = 0;
    if(printConsole){
        printf("Memoria -- Type -- Operacion -- promedio -- total \n");
    }
    log_info(file_log,"Memoria -- Type -- Operacion -- promedio -- total");
    for (i = 0; i < listaMemorias->elements_count ; ++i) {
        metricaMemoria = list_get(listaMemorias,i);
        for (k = 0; k < 2 ; ++k) {
            for (j = 0; j < metricaMemoria->tipos->elements_count; ++j) {
                criterio = list_get(metricaMemoria->tipos, j);
                promerio = promedioBYOperation(metricaMemoria->history,*criterio,arrayOperacion[k]);
                total = getTotalBYOperacion(metricaMemoria->history,*criterio,arrayOperacion[k]);
                if(printConsole){
                    printf("   %d   ",metricaMemoria->numeroMemoria);
                    printf("     %s   ", getTipoCriterioBYEnum(*criterio));
                    printf("   %s   ", getOperacionBYEnum(arrayOperacion[k]));
                    printf("     %.2f   ", promerio);
                    printf("     %d   \n", total);
                }
                log_info(file_log,"   %d        %s      %s        %.2f        %d   ",metricaMemoria->numeroMemoria,getTipoCriterioBYEnum(*criterio),getOperacionBYEnum(arrayOperacion[k]),promerio,total);
            }
        }
        destroyMetricaMemoria(metricaMemoria);
    }
    list_destroy(listaMemorias);
}

void pthreadLogMetricas(){
    while (1){
        calcularMetricas(false);
        sleep(30);
    }
}