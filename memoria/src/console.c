// console
#include "console.h"
#include "request.h"
#include "comandos.h"
#include <funcionesCompartidas/listaMetadata.h>

extern t_log *file_log;

st_insert *cargarInsert2(char *comando) {
    char *hayString;
    st_insert *insert = malloc(sizeof(st_insert));
    insert->operacion = INSERT;
    char **listSplit = string_split(comando, "\"");
    char **listSplit2;
    if (listSplit[1] == NULL) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        free(insert);
        return NULL;
    }
    insert->value = strdup(listSplit[1]);
    if (listSplit[2]) {
        insert->timestamp = strtol(listSplit[2], &hayString, 10);
        if (!string_is_empty(hayString)) {
            string_iterate_lines(listSplit, (void *) free);
            free(listSplit);

            free(insert);
            return NULL;
        }
    } else {
        insert->timestamp = obtenerMilisegundosDeHoy();
    }

    listSplit2 = string_split(listSplit[0], " ");
    if (listSplit2[1] == NULL) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        free(insert);
        return NULL;
    }

    insert->nameTable = strdup(listSplit2[1]);
    if (listSplit2[2] == NULL) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        string_iterate_lines(listSplit2, (void *) free);
        free(listSplit2);

        free(insert);
        return NULL;
    }
    insert->key = strtol(listSplit2[2], &hayString, 10);
    if (!string_is_empty(hayString)) {
        string_iterate_lines(listSplit, (void *) free);
        free(listSplit);

        string_iterate_lines(listSplit2, (void *) free);
        free(listSplit2);
        free(insert);
        return NULL;
    }

    string_iterate_lines(listSplit, (void *) free);
    free(listSplit);

    string_iterate_lines(listSplit2, (void *) free);
    free(listSplit2);
    return insert;
}

void makeCommand(char *command){
  int typeCommand = getEnumFromString(command);

  switch(typeCommand){
  	  case INSERT:
  		  log_info(file_log, "Ejecutando un insert");
  		  st_insert * insert;
  		  int codigoInsert = 0;
  		  if((insert = cargarInsert2(command))){
              if((codigoInsert = comandoInsert(insert)) == -1){
                  printf("no se pudo hacer el insert\n");
                  log_error(file_log, "no se pudo hacer el insert");
              }
              if(codigoInsert == -2){
                  printf("no hay espacio en las paginas\n");
                  log_error(file_log, "no hay espacio en las paginas");
              }
  		  }
  		  if(insert){
              destroyInsert(insert);
  		  }
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
                  destroyRegistro(registro);
              }else{
                printf("No hay resultados para este select\n");
                log_info(file_log, "No hay resultados para este select");
              }
    	  }else{
              log_error(file_log, "[+] Error en datos de SELECT. \n");
    	  }
    	  destoySelect(select);
    	  break;
      case CREATE:
    	  log_info(file_log, "[+] El comando es un CREATE\n");
    	  st_create * create;
    	  int codigo;

		  if((create = cargarCreate(command))){
			  log_info(file_log, "[+] Ejecutando CREATE\n");
			  codigo = mandarCreate(create);

			  mostrarRespuesta(codigo);
		  }else log_info(file_log, "[+] Error en datos de CREATE\n");

    	  destroyCreate(create);
    	  break;
      case DROP:
    	  log_info(file_log, "[+] El comando es un DROP\n");
    	  st_drop * drop;
    	  int cod;

		  if((drop = cargarDrop(command))){
			  log_info(file_log, "[+] Ejecutando DROP\n");
			  cod = mandarDrop(drop);

			  mostrarRespuesta(cod);
		  }else log_info(file_log, "[+] Error en datos de DROP\n");

    	  destroyDrop(drop);
    	  break;
      case DESCRIBE:
    	  log_info(file_log, "[+] El comando es un DESCRIBE\n");
    	  st_describe * describe;
    	  st_metadata * meta;
    	  t_list * lista;
    	  int respuesta;
    	  if((describe = cargarDescribe(command))){
    		  log_info(file_log, "[+] Ejecutando DESCRIBE\n");
    		  respuesta = mandarDescribe(describe,&meta);

    		  mostrarRespuesta(respuesta);
    		  if(respuesta == 15){
    			  mostrarTabla(meta);
    		  }
    		  destroyDescribe(describe);
    	  }else{
    		  respuesta = mandarDescribeGlobal(&lista);

    		  mostrarRespuesta(respuesta);
    		  if(respuesta == 13){
    			  list_iterate(lista,(void*)mostrarTabla);
    			  destroyListaMetaData(lista);
    		  }
    	  }
  	 	break;
      case JOURNAL:
    	  log_info(file_log, "[+] El comando es un JOURNAL\n");
    	  comandoJournal();
        break;
      case EXIT:
    	  break;
      default: {
          log_info(file_log, "No se reconoce el comando");
      }
  }

}

void console(){
  char *command, *ingreso ;
  size_t tamBuffer = 100;
  printf("[+] Write a LQL command: \n");

  ingreso = malloc(sizeof(char) * tamBuffer);
  getline(&ingreso, &tamBuffer, stdin);
  command = strtok(ingreso, "\n");

  while(strcmp(command,"exit") != 0){
    makeCommand(command);
    printf("------------------------------\n");

    free(ingreso);
    ingreso = malloc(sizeof(char) * tamBuffer);
    printf("[+] Write a LQL command: \n");
    getline(&ingreso, &tamBuffer, stdin);
    command = strtok(ingreso, "\n");
  }
  free(ingreso);
  printf("[-] Exiting console\n" );
}

void mostrarRespuesta(int respuesta){

	switch(respuesta)
	{
		case 4:
			log_error(file_log,"La tabla ingresada no existe");
			printf("El tabla ingresada no existe\n");
			break;
		case 7:
			log_error(file_log,"La tabla ingresada ya existe");
			printf("La tabla ingresada ya existe\n");
			break;
		case 8:
			log_info(file_log, "La tabla se creo correctamente");
			printf("La tabla se creo correctamente\n");
			break;
		case 9:
			log_error(file_log, "La tabla se elimino correctamente");
			printf("La tabla se elimino correctamente\n");
			break;
		case 10:
			log_error(file_log, "No se pudo crear la tabla");
			printf("No se pudo crear la tabla\n");
			break;
		case 12:
			log_error(file_log, "No se pudo realizar la request");
			printf("No se pudo realizar la request\n");
			break;
		case 13:
			log_info(file_log, "Describe de tablas encontradas");
			printf("Describe de tablas encontradas\n");
			break;
		case 15:
			log_info(file_log, "Describe de tabla");
			printf("Describe de tabla\n");
			break;
	}
}

void mostrarTabla(st_metadata * meta){
    printf("\nTable: %s\nConsistency: %s\nPartitions: %d\nCompaction Time: %d\n",meta->nameTable, meta->consistency, meta->partitions, meta->compaction_time);
}
