//
// Created by miguelchauca on 04/05/19.
//

#include "threadPlanificador.h"

t_list * listProcesos;
sem_t colaDeListos;
extern t_log * file_log;
extern config *configuracion;

void showAllProceso(){
    int i,j;
    for (i = 0; i < listProcesos->elements_count ; ++i) {
        t_list * procesoInstrucciones = list_get(listProcesos,i);
        printf("proceso numero [%d]\n",i);
        for (j = 0; j < procesoInstrucciones->elements_count; ++j) {
            stinstruccion * instruccion = list_get(procesoInstrucciones,j);
            printf("instrucion numero [%d]\n",instruccion->operacion);
        }
    }
}

t_list * tomarInstrucciones(t_list * proceso){
    if(proceso->elements_count > configuracion->QUANTUM){
        return list_take_and_remove(proceso,configuracion->QUANTUM);
    }else{
        return list_take_and_remove(proceso,proceso->elements_count);
    }
}

void atenderRequest(){
    int i;
    stinstruccion * instruccionScript;
    st_memoria * datomemoria;
    while (1){
        sem_wait(&colaDeListos);
        log_info(file_log,"atendiendo nuevo proceso");
        //optener el proceso y sacarlo de la lista
        t_list * proceso = list_remove(listProcesos,0);
        //tomamos y removemos las instruciones del proceso
        t_list * instrucciones = tomarInstrucciones(proceso);
        //ejecutamos esas instruciones tomadas
        for (i = 0; i < instrucciones->elements_count; ++i) {
            instruccionScript = list_get(instrucciones,i);
            datomemoria = getMemoria(instruccionScript->criteio, instruccionScript->tag);
            if(datomemoria){
                enviarRequestMemoria(instruccionScript,datomemoria);
            }else{
                printf("no hay memoria disponible\n");
            }
        }
        sleep(6);
        //validar resultado de las 4 instruciones

        //verificar si quedan instrucciones para volver a ponerlo en la cola o terminarlo
        if(proceso->elements_count > 0){
            cargarNuevoScript(proceso);
        }
    }
}

void * inicialPlanificador(){
    listProcesos = list_create();
    sem_init(&colaDeListos,0,0);
    atenderRequest();
}

void cargarNuevoScript(t_list * listInstrucciones){
    list_add(listProcesos,listInstrucciones);
    sem_post(&colaDeListos);
}

