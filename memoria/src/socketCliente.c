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

    void* mensaje = createMessage(&request, paqueteDatos);
    enviar_message(fdFileSystem, mensaje, file_log, &control);

    if(control == 0){
        paqueteDeRespuesta = getMessage(fdFileSystem, &respuesta, &control);
        if(respuesta.codigo != 14){
            return NULL;
        }else{
            st_registro* registro = deserealizarRegistro(paqueteDeRespuesta);
            return registro;
        }
    } else {
        log_error(file_log, "Fallo el envio del Select");
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






