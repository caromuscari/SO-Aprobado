// console
#include "console.h"

void makeCommand(char *command){
  int typeCommand = getEnumFromString(command);
  switch(typeCommand)
		{
			case INSERT:{
				printf("[+] I got INSERT.");
        st_insert * insert;
        if((insert = cargarInsert(command))){
          //createInstruccList(insert,INSERT);
          printf("[+] Executing INSERT");
          sleep(1);
        }break;
      }
      case SELECT:{
        printf("[+] I got SELECT\n");
        st_select * select;
        if((select = cargarSelect(command))){
          printf("[+] Executing SELECT.\n");
          sleep(1);
        }break;
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
  printf("[-] Exiting console\n" );
}
