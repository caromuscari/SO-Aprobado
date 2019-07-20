#include "journal.h"
#include "comandos.h"

extern t_configuracionMemoria *configMemoria;

void * hiloJournal(){
	while(1){
		comandoJournal();
		sleep(configMemoria->TIEMPO_JOURNAL/1000);
	}
}

