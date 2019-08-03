// console
#include "console.h"
#include "request.h"
#include "comandos.h"
#include <funcionesCompartidas/listaMetadata.h>
#include <readline/readline.h>

extern t_log *file_log;
extern t_configuracionMemoria* configMemoria;

void makeCommand(char *command) {
    int typeCommand = getEnumFromString(command);

    switch (typeCommand) {
        case INSERT:
            log_info(file_log, "Ejecutando un Insert");
            st_insert *insert;
            int codigoInsert = 0;
            if ((insert = cargarInsert(command))) {
                if ((codigoInsert = comandoInsert(insert)) == MAYORQUEVALUEMAX) {
                    printf("El value es mayor al tamaño maximo\n");
                }
                if (codigoInsert == FULLMEMORY) {
                    printf("No hay espacio en las paginas, realizando Journal\n");
                    log_error(file_log, "No hay espacio en las paginas");
                    comandoJournal();
                    printf("Journal terminado, realice el Insert nuevamente\n");
                }
                if (codigoInsert == OK) {
                    printf("Se realizo correctamente el Insert\n");
                }
            } else {
                printf("Verificar datos\n");
                log_error(file_log, "No se pudo cargar el comando Insert");
            }
            if (insert) {
                destroyInsert(insert);
            }
            break;
        case SELECT:
            log_info(file_log, "Ejecutando Select");
            st_select *select;
            enum_resultados resultado = NOOK;
            st_registro *registro;
            if ((select = cargarSelect(command))) {
                registro = comandoSelect(select, &resultado);
                switch (resultado){
                	case OK:{
                		printf("value [%s]\n", registro->value);
                		destroyRegistro(registro);
                		break;
                	}
                	case NOOK:{
                		printf("No hay resultados para este select\n");
                		log_info(file_log, "No hay resultados para este select");
                		break;
                	}
                	case FULLMEMORY: {
                		printf("Memoria llena, realizando Journal \n");
                		comandoJournal();
                		printf("Journal terminado, realice el Select nuevamente\n");
                		break;
                	}
                }

                destoySelect(select);
            } else {
            	printf("Verificar datos\n");
                log_error(file_log, "Error en datos de Select");
            }

            break;
        case CREATE:
            log_info(file_log, "Ejecutando Create");
            st_create *create;
            int codigo;

            if ((create = cargarCreate(command))) {
                codigo = mandarCreate(create);
                if (codigo != NOOK) {
                    mostrarRespuesta(codigo);

                }
                destroyCreate(create);
            } else {
            	printf("Verificar datos\n");
            	log_info(file_log, "Error en datos de Create");
            }

            break;
        case DROP:
            log_info(file_log, "Ejecutando Drop");
            st_drop *drop;
            int cod;

            if ((drop = cargarDrop(command))) {
                cod = mandarDrop(drop);
                if (cod != NOOK) {
                    mostrarRespuesta(cod);
                    if (cod == 9) {
                        removerSegmentoPorNombrePagina(drop->nameTable);
                    }
                } else {
			      printf("No se pudo relizar el drop\n");
                }
                destroyDrop(drop);
            } else {
            	printf("Verificar datos\n");
            	log_error(file_log, "Error en datos de DROP");
            }

            break;
        case DESCRIBE: {
            st_describe *describe;
            t_list *lista;
            st_messageResponse *respuesta;
            if ((describe = cargarDescribe(command))) {
                //describe
                log_info(file_log, "Ejecutando Describe");
                respuesta = mandarDescribe(describe);
                if (respuesta) {
                    mostrarRespuesta(respuesta->cabezera.codigo);
                    if (respuesta->cabezera.codigo == 15) {
                        size_t size = 0;
                        st_metadata *meta = deserealizarMetaData(respuesta->buffer, &size);
                        mostrarTabla(meta);
                        destroyMetaData(meta);
                    }
                    destroyStMessageResponse(respuesta);
                } else {
                    printf("Fallo al recibir respuesta del describe\n");
                }
                destroyDescribe(describe);
            } else {
                //describe global
                log_info(file_log, "Ejecutando Describe Global");
                respuesta = mandarDescribeGlobal();
                if (respuesta) {
                    mostrarRespuesta(respuesta->cabezera.codigo);
                    if (respuesta->cabezera.codigo == 13) {
                        lista = deserealizarListaMetaData(respuesta->buffer, respuesta->cabezera.sizeData);
                        list_iterate(lista, (void *) mostrarTabla);
                        destroyListaMetaData(lista);
                    }
                    destroyStMessageResponse(respuesta);
                } else {
                    printf("Fallo al recibir respuesta del Describe Global\n");
                }
            }
            break;
        }
        case JOURNAL:
            log_info(file_log, "Ejecutando Journal");
            comandoJournal();
            break;
        case EXIT:{
        	return;
            break;
        }
        default: {
        	printf("Verificar datos\n");
            log_info(file_log, "No se reconoce el comando");
        }
    }
sleep(configMemoria->RETARDO_FS/1000);
}



void console() {
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

void mostrarRespuesta(int respuesta) {

    switch (respuesta) {
        case 4:
            log_error(file_log, "La tabla ingresada no existe");
            printf("El tabla ingresada no existe\n");
            break;
        case 7:
            log_error(file_log, "La tabla ingresada ya existe");
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

void mostrarTabla(st_metadata *meta) {
    printf("\nTable: %s\nConsistency: %s\nPartitions: %d\nCompaction Time: %d\n", meta->nameTable, meta->consistency,
           meta->partitions, meta->compaction_time);
}
