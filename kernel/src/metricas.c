//
// Created by miguel on 27/7/19.
//

#include "metricas.h"

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

void calcularMetricas(){
    double now = obtenerMilisegundosDeHoy();
    int i,j,k;
    st_metrica_memoria * metricaMemoria = NULL;
    TypeCriterio * criterio;
    enum OPERACION arrayOperacion[] = {INSERT,SELECT};
    t_list * listaMemorias = getHistoryByRange((now-30000),now);
    printf("Memoria -- Type -- Operacion -- promedio -- total \n");
    for (i = 0; i < listaMemorias->elements_count ; ++i) {
        metricaMemoria = list_get(listaMemorias,i);
        for (k = 0; k < 2 ; ++k) {
            for (j = 0; j < metricaMemoria->tipos->elements_count; ++j) {
                printf("   %d   ",metricaMemoria->numeroMemoria);
                criterio = list_get(metricaMemoria->tipos, j);
                printf("     %s   ", getTipoCriterioBYEnum(*criterio));
                printf("   %s   ", getOperacionBYEnum(arrayOperacion[k]));
                printf("     %.2f   ",promedioBYOperation(metricaMemoria->history,*criterio,arrayOperacion[k]));
                printf("     %d   \n", getTotalBYOperacion(metricaMemoria->history,*criterio,arrayOperacion[k]));
            }
        }
        destroyMetricaMemoria(metricaMemoria);
    }
    list_destroy(listaMemorias);
}