// console
#include "console.h"

extern t_log *file_log;

void makeCommand(char *command){
  int typeCommand = getEnumFromString(command);
  switch(typeCommand){
		case INSERT:{
				printf("[+] I got INSERT.");
        st_insert * insert;
        if((insert = cargarInsert(command))){
        	comandoInsert(insert);
        	printf("[+] Executing INSERT");
        	sleep(1);
        }
          break;
      }
      case SELECT:{
        log_info(file_log, "[+] El comando es un SELECT\n");
        st_select * select;
        if((select = cargarSelect(command))){
        	log_info(file_log, "[+] Ejecutando SELECT.\n");
        	comandoSelect(select);
        	sleep(1);
       }

       log_info(file_log, "[+] No se pudo ejecutar el SELECT. \n");
       break;
      }

      case CREATE:{

      break;
      }

      case DROP:{

      break;
      }
      case DESCRIBE:{

  	  break;
      }
     /* case JOURNAL:{

            break;
            }
  */
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
  printf("[-] Exiting console\n" );
}
