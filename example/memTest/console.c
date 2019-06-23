// console
#include "console.h"



//comandoInsert(punteroTabla,command[1],command[2],command[3])
  //INSERT [NOMBRE_TABLA] [KEY] “[VALUE]”
  //INSERT TABLA2 4 "giuliana"



//	cargarPagina(punteroTabla,0,1,"hola");


//mostrarPagina(tablaDePaginas,1);



void makeCommand(char *command,st_pagina *punteroTabla){
  int typeCommand = getEnumFromString(command);
  switch(typeCommand){
		case INSERT:{
				printf("[+] I got INSERT. \n");
        st_insert * insert;
        if((insert = cargarInsert(command))){

          printf("%s\n",insert->value);
          comandoInsert(punteroTabla,insert->key,insert->nameTable,insert->value);
          //createInstruccList(insert,INSERT);
          printf("[+] Executing INSERT");
        }
          break;
      }
      case SELECT:{
        printf("[+] I got SELECT\n");
        st_select * select;
        if((select = cargarSelect(command))){
          comandoSelect(punteroTabla,select->key);
          printf("[+] Executing SELECT.\n");
          }
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

void console(st_pagina *punteroTabla){

  char *command ;
  printf("[+] Write a LQL command: \n");
  command = readline("[>] ");
  while(strcmp(command,"exit") != 0){
    makeCommand(command,punteroTabla);
    free(command);
    printf("------------------------------\n");
    command = readline("[>] ");
  }
  printf("[-] Exiting console\n" );
}
