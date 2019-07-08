// console
#include "console.h"

extern t_log *file_log;

void makeCommand(char *command){
  int typeCommand = getEnumFromString(command);

  switch(typeCommand){
  	  case INSERT:
  		  log_info(file_log, "Ejecutando un insert");
  		  st_insert * insert;
  		  if((insert = cargarInsert(command))){
              if(comandoInsert(insert) < 0){
                  log_error(file_log, "no se puedo hacer el insert");
              }
  		  }
  		  destroyInsert(insert);
  		  break;
      case SELECT:
    	  log_info(file_log, "[+] El comando es un SELECT\n");
    	  st_select * select;
    	  st_registro * registro;
    	  if((select = cargarSelect(command))){
    		  log_info(file_log, "[+] Ejecutando SELECT.\n");
              registro = comandoSelect(select);
              if(registro){
                  printf("value [%s]\n", registro->value);
              }else{
                log_info(file_log, "No se encontro el select");
              }
    	  }else{
              log_error(file_log, "[+] Error en datos de SELECT. \n");
    	  }
    	  destoySelect(select);
    	  break;
      case CREATE:
    	  log_info(file_log, "[+] El comando es un CREATE\n");
    	  st_create * create;
		  if((create = cargarCreate(command))){
			  log_info(file_log, "[+] Ejecutando CREATE\n");
			  //Hacer create
		  }
		  log_info(file_log, "[+] Error en datos de CREATE\n");

    	  destroyCreate(create);
    	  break;
      case DROP:
    	  log_info(file_log, "[+] El comando es un DROP\n");
    	  st_drop * drop;
		  if((drop = cargarDrop(command))){
			  log_info(file_log, "[+] Ejecutando DROP\n");
			  //Hacer drop
		  }
		  log_info(file_log, "[+] Error en datos de DROP\n");

    	  destroyDrop(drop);
    	  break;
      case DESCRIBE:
    	  log_info(file_log, "[+] El comando es un DESCRIBE\n");
    	  st_describe * describe;
    	  if((describe = cargarDescribe(command))){
    		  log_info(file_log, "[+] Ejecutando DESCRIBE\n");
    		  //Hacer describe
    	  }
    	  log_info(file_log, "[+] Error en datos de DESCRIBE\n");

    	  destroyDrop(drop);
  	 	break;
      case JOURNAL:
    	  log_info(file_log, "[+] El comando es un JOURNAL\n");
    	  //st_drop * drop;
    	  /*if((drop = cargarDrop(command))){
    		  log_info(file_log, "[+] Ejecutando JOURNAL\n");
    		  //Hacer drop
    	  }*/
    	  //log_info(file_log, "[+] Error en datos de JOURNAL\n");

    	  //destroyDrop(drop);
        break;
      case EXIT:
    	  break;
      default: {
          log_info(file_log, "No se reconoce el comando");
      }
  }

}

void console(){
  char *command ;
  printf("[+] Write a LQL command: \n");
  command = readline("[>] ");
  while(strcmp(command,"exit") != 0){
    makeCommand(command);
    free(command);
    printf("------------------------------\n");
    command = readline("[>] ");
  }
  free(command);
  printf("[-] Exiting console\n" );
}
