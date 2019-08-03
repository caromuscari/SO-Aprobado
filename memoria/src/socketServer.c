#include "socketServer.h"
#include "request.h"

extern t_log *file_log;
extern t_configuracionMemoria * configMemoria;
#include <funcionesCompartidas/API.h>
#include <funcionesCompartidas/listaMetadata.h>
#include <commons/collections/list.h>
extern int loop;

void atenderMensaje(int * fdClient){
    int control = 0;
    header request;
    size_t sizePaqueteRes = 0;
    void * buffer = NULL;
    void * paqueteDeRespuesta = getMessage(*fdClient,&request,&control);
    if (paqueteDeRespuesta == NULL) {
        log_error(file_log, "Fallo la conexion con el kernel");
        close(*fdClient);
        free(fdClient);
        //pthread_exit(NULL);
    }
    switch (request.codigo) {
    	case INSERT: {
    		st_insert *insert = desserealizarInsert(paqueteDeRespuesta);
    		log_info(file_log, "El comando es un Insert");
    		log_info(file_log, "Table [%s]", insert->nameTable);
    		log_info(file_log, "Key [%d]", insert->key);
    		log_info(file_log, "Value [%s]", insert->value);

    		buffer = strdup("1");

    		int resultado = comandoInsert(insert);
    		if(resultado == MAYORQUEVALUEMAX){
    			enviarRespuesta(NOSUCCESS, buffer, *fdClient, &control, strlen(buffer));
    		}
    		if(resultado == FULLMEMORY){
    			//memoria full
    			enviarRespuesta(MEMORIAFULL, buffer, *fdClient, &control, strlen(buffer));
    		}else {
    			enviarRespuesta(SUCCESS, buffer, *fdClient, &control, strlen(buffer));
    		}
    		destroyInsert(insert);
    		free(buffer);
    		break;
    	}
        case SELECT: {
            st_select * select = deserealizarSelect(paqueteDeRespuesta);
            enum_resultados resultado = NOOK;

            st_registro* registro = comandoSelect(select, &resultado);
            switch (resultado){
				case OK:{
					buffer = serealizarRegistro(registro, &sizePaqueteRes);
					enviarRespuesta(SUCCESS, buffer, *fdClient, &control, sizePaqueteRes);
					break;
				}
				case NOOK:{
					buffer = strdup("1");
					enviarRespuesta(NOSUCCESS, buffer, *fdClient, &control, strlen(buffer));
					break;
				}
				case FULLMEMORY: {
					buffer = strdup("1");
					enviarRespuesta(MEMORIAFULL, buffer, *fdClient, &control, strlen(buffer));
					break;
				}
			}
            destoySelect(select);
            free(buffer);
            break;
        }
    	case CREATE:{
            st_create * create = deserealizarCreate(paqueteDeRespuesta);
            int respuesta;
            log_info(file_log, "El comando es un Create");

            respuesta = mandarCreate(create);

            buffer = strdup("1");

            if(respuesta == 8) {
            	enviarRespuesta(SUCCESS, buffer, *fdClient, &control, strlen(buffer));
            } else {
            	enviarRespuesta(NOSUCCESS, buffer, *fdClient, &control, strlen(buffer));
            }

            destroyCreate(create);
            free(buffer);
            break;
    	}
    	case DROP:{
            st_drop * drop = deserealizarDrop(paqueteDeRespuesta);
            int respuesta;

            respuesta = mandarDrop(drop);
            buffer = strdup("1");

            if(respuesta == 9){
            	enviarRespuesta(SUCCESS, buffer, *fdClient, &control, strlen(buffer));
                removerSegmentoPorNombrePagina(drop->nameTable);
            }
            else enviarRespuesta(NOSUCCESS, buffer, *fdClient, &control, strlen(buffer));

            destroyDrop(drop);
            free(buffer);
            break;
    	}
    	case DESCRIBE:{
    		st_messageResponse* respuesta;
            st_describe * describe = deserealizarDescribe(paqueteDeRespuesta);
            log_info(file_log, "El comando es un Describe");

            respuesta = mandarDescribe(describe);
            if(respuesta){
            	 if(respuesta->cabezera.codigo == 15){
            		enviarRespuesta(SUCCESS,respuesta->buffer,*fdClient,&control,respuesta->cabezera.sizeData);
            	 }else{
            		buffer = strdup("1");
            		enviarRespuesta(NOSUCCESS,buffer,*fdClient,&control,strlen(buffer));
                    free(buffer);
            	 }

                 destroyStMessageResponse(respuesta);
            } else {
            	buffer = strdup("1");
            	enviarRespuesta(NOSUCCESS, buffer, *fdClient, &control, strlen(buffer));
            	free(buffer);
            }
            destroyDescribe(describe);
            break;
    	}
    	case DESCRIBEGLOBAL:{
    		st_messageResponse* respuesta;
    		log_info(file_log, "El comando es un Describe Global");
    		respuesta = mandarDescribeGlobal();
    		if(respuesta){
    			if(respuesta->cabezera.codigo == 13){
    				enviarRespuesta(SUCCESS, respuesta->buffer, *fdClient, &control, respuesta->cabezera.sizeData);
    			}else{
    				buffer = strdup("1");
    				enviarRespuesta(NOSUCCESS, buffer, *fdClient, &control, strlen(buffer));
    				free(buffer);
    			}
    			destroyStMessageResponse(respuesta);
    		} else {
				buffer = strdup("1");
				enviarRespuesta(NOSUCCESS, buffer, *fdClient, &control, strlen(buffer));
				free(buffer);
    		}
    		break;
    	}
        case BUSCARTABLAGOSSIPING: {
            st_data_memoria * dataMemoria = deserealizarMemoria(paqueteDeRespuesta, request.sizeData);
            actualizarListaMemorias(dataMemoria);
            list_destroy(dataMemoria->listaMemorias);
            free(dataMemoria);
            void *paqueteLista = devolverListaMemoria(&sizePaqueteRes);
            enviarRespuesta(SUCCESS,paqueteLista,*fdClient,&control,sizePaqueteRes);
            log_info(file_log,"Realizando gossiping");
            free(paqueteLista);
            break;
        }
    	case JOURNAL:{
    		log_info(file_log, "Realizando Journal");
    		int respuesta = comandoJournal();
    		buffer = strdup("1");
    		if(respuesta == OK){
                enviarRespuesta(SUCCESS, buffer, *fdClient, &control, strlen(buffer));
    		} else {
                enviarRespuesta(NOSUCCESS, buffer, *fdClient, &control, strlen(buffer));
    		}
    		free(buffer);
    		break;
    	}
    }

    free(paqueteDeRespuesta);
    close(*fdClient);
    free(fdClient);
//    pthread_exit(NULL);
}

void * start_server() {
	int control = 0;
	int socketServer = makeListenSock(configMemoria->PUERTO, file_log, &control);
	if(control != 0){
	    log_error(file_log,"[server] no se pudo inicializar el server");
		pthread_exit(NULL);
	}
	int  * fdClient;
	while (loop){
        fdClient = malloc(sizeof(int));
        *fdClient = aceptar_conexion(socketServer, file_log, &control);
        if (control != 0) {
            log_error(file_log, "No se puede aceptar la conexion");
            continue;
        }
        //pthread_create(&nuevoCliente,NULL,(void *)atenderMensaje,fdClient);
        //pthread_detach(nuevoCliente);
        atenderMensaje(fdClient);
        sleep(configMemoria->RETARDO_FS/1000);
	}
	pthread_exit(NULL);
}

void enviarRespuesta(int codigo, char * buffer, int socketC, int * status, size_t tam){

	header * head = malloc(sizeof(header));

	head->letra = 'M';
	head->codigo = codigo;
	head->sizeData = tam;

	message * mensaje = createMessage(head, buffer);

	enviar_message(socketC, mensaje, file_log, status);
	if(*status != 0){
	    log_error(file_log,"No se pudo enviar una respuesta al kernel");
	}

	free(head);
	free(mensaje->buffer);
	free(mensaje);
}
