#include "journal.h"
#include "comandos.h"

extern t_configuracionMemoria *configMemoria;
extern t_log *file_log;

void * hiloJournal(){
	while(1){
		log_info(file_log, "[theadJournal]Iniciando hilo Journal");
		comandoJournal();
		log_info(file_log, "[theadJournal]Termino hilo Journal");
		sleep(configMemoria->TIEMPO_JOURNAL/1000);
 	}
	pthread_exit(NULL);
}

