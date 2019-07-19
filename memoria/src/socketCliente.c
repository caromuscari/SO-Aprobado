#include "socketCliente.h"

extern t_log *file_log;

extern t_configuracionMemoria * configMemoria;
extern int fdFileSystem;

// CLIENTE


st_registro* obtenerSelect(st_select * comandoSelect){
	int control = 0;
	header request;
	void* paqueteDeRespuesta;
	header respuesta;
    request.letra = 'M';
    request.codigo = SELECT;

    size_t size;
    void* paqueteDatos = serealizarSelect(comandoSelect, &size);

    request.sizeData = size;

    message* mensaje = createMessage(&request, paqueteDatos);
    enviar_message(fdFileSystem, mensaje, file_log, &control);
    free(paqueteDatos);
    free(mensaje->buffer);
    free(mensaje);
    if(control != 0){
    	log_error(file_log, "No se pudo enviar el mensaje del select");
    	return NULL;
    }

    paqueteDeRespuesta = getMessage(fdFileSystem, &respuesta, &control);
    if(paqueteDeRespuesta== NULL){
    	log_error(file_log, "Fallo la conexion con el File System");
    	return NULL;
    }
    if(respuesta.codigo == 14){
    	return deserealizarRegistro(paqueteDeRespuesta);
    } else {
    	return NULL;
    }
}

void* informarDrop(st_drop* comandoDrop){
	int control = 0;
	header request, respuesta;
	void* paqueteDeRespuesta;

	int socketCliente = establecerConexion(configMemoria->IP_FS, configMemoria->PUERTO_FS, file_log, &control);
	if(socketCliente != -1){
		request.letra = 'M';
		request.codigo = DROP;

		size_t size;
		void* paqueteDatos = serealizarDrop(comandoDrop, &size);

		request.sizeData = size;

		void* mensaje = createMessage(&request, paqueteDatos);
		enviar_message(socketCliente, mensaje, file_log, &control);


	}
	return NULL;
}






