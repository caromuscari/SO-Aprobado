#include "journal.h"
#include "comandos.h"

extern t_configuracionMemoria *configMemoria;
extern t_log *file_log;

void * hiloJournal(){
	while(1){
		log_info(file_log, "Iniciando hilo Journal");
		comandoJournal();
		log_info(file_log, "Termino hilo Journal");
		sleep(configMemoria->TIEMPO_JOURNAL/1000);
 	}
}

