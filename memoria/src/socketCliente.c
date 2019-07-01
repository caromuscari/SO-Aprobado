#include "socketCliente.h"

extern t_log *file_log;

extern t_configuracionMemoria configMemoria;

// CLIENTE


//cambiar esto a void* , ahora lo saco para que compile
void obtenerSelect(st_select * comandoSelect){
	int control = 0;
	header request;
	void* paqueteDeRespuesta;
	header respuesta;
	int socketCliente = establecerConexion(configMemoria.ipFS, configMemoria.puertoFS, file_log, &control);
	if(socketCliente != -1){
		request.letra = 'M';
		request.codigo = SELECT;

		size_t size;
		void* paqueteDatos = serealizarSelect(comandoSelect, &size);

		request.sizeData = size;

		void* mensaje = createMessage(&request, paqueteDatos);
		enviar_message(socketCliente, mensaje, file_log, &control);

		if(control == 0){
			paqueteDeRespuesta = getMessage(socketCliente, &respuesta, &control);

			// deserealizar ---> time key value
		}
	}
}







