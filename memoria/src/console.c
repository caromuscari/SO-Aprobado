// console
#include "console.h"
#include "request.h"
#include "comandos.h"
#include <funcionesCompartidas/listaMetadata.h>
#include <readline/readline.h>

extern t_log *file_log;

void makeCommand(char *command){
  int typeCommand = getEnumFromString(command);

  switch(typeCommand){
  	  case INSERT:
  		  log_info(file_log, "Ejecutando un insert");
  		  st_insert * insert;
  		  int codigoInsert = 0;
  		  if((insert = cargarInsert(command))){
              if((codigoInsert = comandoInsert(insert)) == MAYORQUEVALUEMAX){
                  printf("El value es mayor al tamaño maximo\n");
              }
              if(codigoInsert == FULLMEMORY){
                  printf("No hay espacio en las paginas\n");
                  log_error(file_log, "no hay espacio en las paginas");
              }
  		  } else {
  			  printf("Verifique los parametros\n");
  			  log_error(file_log, "No se pudo cargar el comando Insert");
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
              destoySelect(select);
    	  }else{
              log_error(file_log, "Error en datos de SELECT. \n");
    	  }

    	  break;
      case CREATE:
    	  log_info(file_log, "[+] El comando es un CREATE\n");
    	  st_create * create;
    	  int codigo;

		  if((create = cargarCreate(command))){
			  codigo = mandarCreate(create);
			  if(codigo != NOOK && codigo != SOCKETDESCONECTADO){
				  mostrarRespuesta(codigo);

			  }
			  destroyCreate(create);
		  }else log_info(file_log, "[+] Error en datos de CREATE\n");

    	  break;
      case DROP:
    	  log_info(file_log, "[+] El comando es un DROP\n");
    	  st_drop * drop;
    	  int cod;

		  if((drop = cargarDrop(command))){
			  cod = mandarDrop(drop);
			  if(cod != NOOK && cod != SOCKETDESCONECTADO){
				  mostrarRespuesta(cod);
				  if(cod == 9){
					  removerSegmentoPorNombrePagina(drop->nameTable);
				  }
			  }
			  destroyDrop(drop);
		  }else log_error(file_log, "Error en datos de DROP\n");

    	  break;
      case DESCRIBE:{
    	  st_describe * describe;
    	  t_list * lista;
    	  st_messageResponse* respuesta;
    	  if((describe = cargarDescribe(command))){
    		  //describe
    		  log_info(file_log, "Ejecutando Describe\n");
    		  respuesta = mandarDescribe(describe);
    		  if(respuesta){
    			  mostrarRespuesta(respuesta->cabezera.codigo);
    			  if(respuesta->cabezera.codigo == 15){
    				  size_t size = 0;
    				  st_metadata* meta = deserealizarMetaData(respuesta->buffer, &size);
    				  mostrarTabla(meta);
    				  destroyMetaData(meta);
    		      }
    		      destroyStMessageResponse(respuesta);
    		  } else {
    			  printf("Fallo al recibir respuesta del describe\n");
    		  }
    		  destroyDescribe(describe);
    	  }else{
    		  //describe global
    		  log_info(file_log, "Ejecutando Describe Global\n");
    		  respuesta = mandarDescribeGlobal();
    		  if(respuesta){
    			  mostrarRespuesta(respuesta->cabezera.codigo);
    			  if(respuesta->cabezera.codigo == 13){
    				  lista = deserealizarListaMetaData(respuesta->buffer, respuesta->cabezera.sizeData);
    				  list_iterate(lista,(void*)mostrarTabla);
    				  destroyListaMetaData(lista);
    			  }
    			  destroyStMessageResponse(respuesta);
    		  } else {
    			  printf("Fallo al recibir respuesta del Describe Global\n");
    		  }
    	}
  	 	break;}
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
	char *comando;
	printf("Ingrese comando LQL\n");
	comando = readline(">");
	string_trim(&comando);
	while (strcmp(comando, "exit") != 0) {
		makeCommand(comando);
		free(comando);
		comando = readline(">");
		string_trim(&comando);
	}
	free(comando);
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
			log_error(file_log, "No se encontro la tabla");
			printf("No se encontro la tabla\n");
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
